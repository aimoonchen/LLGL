/*
 * SpirvReflect.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SpirvReflect.h"
#include "SpirvModule.h"
#include "../../Core/CoreUtils.h"
#include <string>


namespace LLGL
{


/*
 * SpirvReflect class
 */

SpirvResult SpirvReflect::Parse(const SpirvModuleView& module)
{
    /* Parse SPIR-V header */
    SpirvHeader header;
    SpirvResult result = module.ReadHeader(header);
    if (result != SpirvResult::Success)
        return result;

    idBound_ = header.idBound;
    names_.Reset(header.idBound);

    /* Parse each SPIR-V instruction in the module */
    for (SpirvInstruction instr : module)
    {
        if (instr.opcode == spv::Op::OpFunction)
        {
            /* No more declarations and decorations after first OpFunction instruction */
            break;
        }

        result = ParseInstruction(instr);
        if (result != SpirvResult::Success)
            return result;
    }

    return SpirvResult::Success;
}

static void ParseSpvExecutionMode(const SpirvInstruction& instr, SpirvReflect::SpvExecutionMode& outExecutionMode)
{
    auto mode = static_cast<spv::ExecutionMode>(instr.GetUInt32(1));
    switch (mode)
    {
        case spv::ExecutionMode::EarlyFragmentTests:
            outExecutionMode.earlyFragmentTest = true;
            break;

        case spv::ExecutionMode::OriginUpperLeft:
            outExecutionMode.originUpperLeft = true;
            break;

        case spv::ExecutionMode::DepthGreater:
            outExecutionMode.depthGreater = true;
            break;

        case spv::ExecutionMode::DepthLess:
            outExecutionMode.depthLess = true;
            break;

        case spv::ExecutionMode::LocalSize:
            outExecutionMode.localSizeX = instr.GetUInt32(2);
            outExecutionMode.localSizeY = instr.GetUInt32(3);
            outExecutionMode.localSizeZ = instr.GetUInt32(4);
            break;

        default:
            break;
    }
}

SpirvResult SpirvReflect::ParseExecutionMode(const SpirvModuleView& module, SpvExecutionMode& outExecutionMode)
{
    /* Parse SPIR-V header */
    SpirvHeader header;
    SpirvResult result = module.ReadHeader(header);
    if (result != SpirvResult::Success)
        return result;

    /* Parse each SPIR-V instruction in the module */
    bool firstModeParsed = false;

    for (SpirvInstruction instr : module)
    {
        if (instr.opcode == spv::Op::OpExecutionMode)
        {
            ParseSpvExecutionMode(instr, outExecutionMode);
            firstModeParsed = true;
        }
        else if (firstModeParsed)
        {
            /* Stop parsing after we found all execution modes */
            break;
        }
    }

    return SpirvResult::Success;
}

static spv::Id FindGlobalPushConstantVariableType(const SpirvModuleView& module)
{
    for (SpirvInstruction instr : module)
    {
        if (instr.opcode == spv::Op::OpVariable)
        {
            /* OpVariable ResultType ResultId StorageClass[0] (Initializer[1]) */
            auto storage = static_cast<spv::StorageClass>(instr.GetUInt32(0));
            if (storage == spv::StorageClass::PushConstant)
            {
                /* Return variable type; Must be OpTypePointer for push constants */
                return instr.type;
            }
        }
        else if (instr.opcode == spv::Op::OpFunction)
        {
            /* No more declarations and decorations after first OpFunction instruction */
            break;
        }
    }
    return 0;
}

static spv::Id FindPointerTypeSubtype(const SpirvModuleView& module, spv::Id pointerTypeId)
{
    for (SpirvInstruction instr : module)
    {
        if (instr.opcode == spv::Op::OpTypePointer)
        {
            if (instr.result == pointerTypeId)
            {
                /* OpTypePointer ResultId StorageClass[0] SubTypeId[1] */
                return instr.GetUInt32(1);
            }
        }
        else if (instr.opcode == spv::Op::OpFunction)
        {
            /* No more declarations and decorations after first OpFunction instruction */
            break;
        }
    }
    return 0;
}

SpirvResult SpirvReflect::ParsePushConstants(const SpirvModuleView& module, SpvBlock& outBlock)
{
    /* Parse SPIR-V header */
    SpirvHeader header;
    SpirvResult result = module.ReadHeader(header);
    if (result != SpirvResult::Success)
        return result;

    /* Find global variable declaration with PushConstant storage class */
    const spv::Id pushConstantVarId = FindGlobalPushConstantVariableType(module);
    if (pushConstantVarId == 0)
        return SpirvResult::Success;

    /* Find pointer subtype for push constant variable */
    const spv::Id pushConstantTypeId = FindPointerTypeSubtype(module, pushConstantVarId);
    if (pushConstantTypeId == 0)
        return SpirvResult::IdTypeMismatch;

    auto GetOrMakeBlockField = [&outBlock](std::uint32_t index) -> SpvBlockField&
    {
        if (index >= outBlock.fields.size())
            outBlock.fields.resize(index + 1);
        return outBlock.fields[index];
    };

    /* Now parse SPIR-V module for block name, its member names, and member offsets */
    for (SpirvInstruction instr : module)
    {
        if (instr.opcode == spv::Op::OpFunction)
        {
            /* No more declarations and decorations after first OpFunction instruction */
            break;
        }

        switch (instr.opcode)
        {
            case spv::Op::OpName:
                /* OpName Target[0] Name[1] */
                if (instr.numOperands < 2)
                    return SpirvResult::OperandOutOfBounds;
                if (instr.GetUInt32(0) == pushConstantTypeId)
                    outBlock.name = instr.GetString(1);
                break;

            case spv::Op::OpMemberName:
                /* OpMemberName TypeId Member[0] Name[1] */
                if (instr.numOperands < 2)
                    return SpirvResult::OperandOutOfBounds;
                if (instr.type == pushConstantTypeId)
                    GetOrMakeBlockField(instr.GetUInt32(0)).name = instr.GetString(1);
                break;

            case spv::Op::OpMemberDecorate:
                /* OpMemberDecorate Target[0] Member[1] Decoration[2] (Values[3+]) */
                if (instr.numOperands < 3)
                    return SpirvResult::OperandOutOfBounds;
                if (instr.GetUInt32(0) == pushConstantTypeId)
                {
                    const auto decoration = static_cast<spv::Decoration>(instr.GetUInt32(2));
                    if (decoration == spv::Decoration::Offset)
                    {
                        if (instr.numOperands < 4)
                            return SpirvResult::OperandOutOfBounds;
                        GetOrMakeBlockField(instr.GetUInt32(1)).offset = instr.GetUInt32(3);
                    }
                }
                break;

            default:
                break;
        }
    }

    return SpirvResult::Success;
}


/*
 * ======= Private: =======
 */

SpirvResult SpirvReflect::ParseInstruction(const SpirvInstruction& instr)
{
    switch (instr.opcode)
    {
        case spv::Op::OpName:
            return OpName(instr);
        case spv::Op::OpDecorate:
            return OpDecorate(instr);
        case spv::Op::OpTypeVoid:
        case spv::Op::OpTypeBool:
        case spv::Op::OpTypeInt:
        case spv::Op::OpTypeFloat:
        case spv::Op::OpTypeVector:
        case spv::Op::OpTypeMatrix:
        case spv::Op::OpTypeImage:
        case spv::Op::OpTypeSampler:
        case spv::Op::OpTypeSampledImage:
        case spv::Op::OpTypeArray:
        case spv::Op::OpTypeRuntimeArray:
        case spv::Op::OpTypeStruct:
        case spv::Op::OpTypeOpaque:
        case spv::Op::OpTypePointer:
        case spv::Op::OpTypeFunction:
            return OpType(instr);
        case spv::Op::OpVariable:
            return OpVariable(instr);
        case spv::Op::OpConstant:
            return OpConstant(instr);
        default:
            return SpirvResult::Success;
    }
}

SpirvResult SpirvReflect::OpName(const Instr& instr)
{
    const spv::Id id = instr.GetUInt32(0);
    if (!(id < idBound_))
        return SpirvResult::IdOutOfBounds;

    names_.Set(id, instr.GetString(1));
    return SpirvResult::Success;
}

SpirvResult SpirvReflect::OpDecorate(const Instr& instr)
{
    const spv::Id id = instr.GetUInt32(0);
    if (!(id < idBound_))
        return SpirvResult::IdOutOfBounds;

    auto decoration = static_cast<spv::Decoration>(instr.GetUInt32(1));
    switch (decoration)
    {
        case spv::Decoration::Binding:
            OpDecorateBinding(instr, id);
            break;
        case spv::Decoration::Location:
            OpDecorateLocation(instr, id);
            break;
        case spv::Decoration::BuiltIn:
            OpDecorateBuiltin(instr, id);
            break;
        default:
            break;
    }
    return SpirvResult::Success;
}

void SpirvReflect::OpDecorateBinding(const Instr& instr, spv::Id id)
{
    auto& variable = uniforms_[id];
    {
        variable.name       = names_[id];
        variable.binding    = instr.GetUInt32(2);
    }
}

void SpirvReflect::OpDecorateLocation(const Instr& instr, spv::Id id)
{
    auto& variable = varyings_[id];
    {
        variable.name       = names_[id];
        variable.location   = instr.GetUInt32(2);
    }
}

void SpirvReflect::OpDecorateBuiltin(const Instr& instr, spv::Id id)
{
    auto& variable = varyings_[id];
    {
        variable.name       = names_[id];
        variable.builtin    = static_cast<spv::BuiltIn>(instr.GetUInt32(2));
    }
}

/*
Example:
  %11 = OpTypeFloat     32                  float
  %12 = OpTypeVector    %11 4               vec4 -> float[4]
  %13 = OpTypeMatrix    %12 4               mat4 -> vec4[4]
  %14 = OpTypeStruct    %13 %13 %12 %11     struct S { mat4; mat4; vec4; float }
  %15 = OpTypePointer   Uniform %14         S*
  %16 = OpVariable      %15 Uniform         uniform S
*/
SpirvResult SpirvReflect::OpVariable(const Instr& instr)
{
    auto storage = static_cast<spv::StorageClass>(instr.GetUInt32(0));

    switch (storage)
    {
        case spv::StorageClass::Uniform:
        case spv::StorageClass::UniformConstant:
        //case spv::StorageClass::PushConstant:
        {
            auto& var = uniforms_[instr.result];
            {
                var.type = FindType(instr.type);
                if (auto structType = var.type->Deref(spv::Op::OpTypeStruct))
                {
                    if (var.name == nullptr || *var.name == '\0')
                        var.name = structType->name;
                    var.size = structType->size;
                }
                else
                    var.size = var.type->size;
            }
        }
        break;

        case spv::StorageClass::Input:
        {
            auto& var = varyings_[instr.result];
            {
                var.type    = FindType(instr.type);
                var.input   = true;
            }
        }
        break;

        case spv::StorageClass::Output:
        {
            auto& var = varyings_[instr.result];
            {
                var.type    = FindType(instr.type);
                var.input   = false;
            }
        }
        break;

        default:
        break;
    }

    return SpirvResult::Success;
}

SpirvResult SpirvReflect::OpConstant(const Instr& instr)
{
    auto& val = constants_[instr.result];
    {
        val.type = FindType(instr.type);

        if (val.type->opcode == spv::Op::OpTypeInt)
        {
            if (val.type->size == 2 || val.type->size == 4)
                val.u32 = instr.GetUInt32(0);
            else if (val.type->size == 8)
                val.u64 = instr.GetUInt64(0);
        }
        else if (val.type->opcode == spv::Op::OpTypeFloat)
        {
            if (val.type->size == 2)
                val.f32 = instr.GetFloat16(0);
            else if (val.type->size == 4)
                val.f32 = instr.GetFloat32(0);
            else if (val.type->size == 8)
                val.f64 = instr.GetFloat64(0);
        }
    }
    return SpirvResult::Success;
}

SpirvResult SpirvReflect::OpType(const Instr& instr)
{
    if (!(instr.result < idBound_))
        return SpirvResult::IdOutOfBounds;

    /* Register type and store it as current type to operate on */
    auto& type = types_[instr.result];
    {
        type.opcode = instr.opcode;
        type.result = instr.result;
        type.name   = names_[instr.result];
    }

    /* Parse respective OpType* instruction */
    #define LLGL_OPTYPE_CASE_HANDLER(NAME)  \
        case spv::Op::NAME:                 \
            NAME(instr, type);              \
            break

    switch (instr.opcode)
    {
        LLGL_OPTYPE_CASE_HANDLER( OpTypeVoid         );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeBool         );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeInt          );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeFloat        );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeVector       );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeMatrix       );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeImage        );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeSampler      );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeSampledImage );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeArray        );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeRuntimeArray );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeStruct       );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeOpaque       );
        LLGL_OPTYPE_CASE_HANDLER( OpTypePointer      );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeFunction     );
        default:
            break;
    }

    #undef LLGL_OPTYPE_CASE_HANDLER

    return SpirvResult::Success;
}

void SpirvReflect::OpTypeVoid(const Instr& /*instr*/, SpvType& type)
{
    // do nothing
}

void SpirvReflect::OpTypeBool(const Instr& /*instr*/, SpvType& type)
{
    type.size = 1;
}

void SpirvReflect::OpTypeInt(const Instr& instr, SpvType& type)
{
    type.size = (instr.GetUInt32(0) / 8);
    type.sign = (instr.GetUInt32(1) != 0);
}

void SpirvReflect::OpTypeFloat(const Instr& instr, SpvType& type)
{
    type.size = (instr.GetUInt32(0) / 8);
}

void SpirvReflect::OpTypeVector(const Instr& instr, SpvType& type)
{
    type.baseType   = FindType(instr.GetUInt32(0));
    type.elements   = instr.GetUInt32(1);
    type.size       = type.baseType->size * type.elements;
}

void SpirvReflect::OpTypeMatrix(const Instr& instr, SpvType& type)
{
    type.baseType   = FindType(instr.GetUInt32(0));
    type.elements   = instr.GetUInt32(1);
    type.size       = type.baseType->size * type.elements;
}

void SpirvReflect::OpTypeImage(const Instr& instr, SpvType& type)
{
    //todo
}

void SpirvReflect::OpTypeSampler(const Instr& instr, SpvType& type)
{
    //todo
}

void SpirvReflect::OpTypeSampledImage(const Instr& instr, SpvType& type)
{
    //todo
}

void SpirvReflect::OpTypeArray(const Instr& instr, SpvType& type)
{
    type.baseType = FindType(instr.GetUInt32(0));
    auto arrayVal = FindConstant(instr.GetUInt32(1));
    type.elements = arrayVal->i32;
}

void SpirvReflect::OpTypeRuntimeArray(const Instr& instr, SpvType& type)
{
    //todo
}

static void AccumulateSizeInVectorBoundary(std::uint32_t& size, std::uint32_t alignment, std::uint32_t appendix)
{
    /* Check if padding must be added first */
    if (size % alignment + appendix > alignment)
        size = GetAlignedSize(size, alignment);

    /* Accumulate next appendix */
    size += appendix;
}

void SpirvReflect::OpTypeStruct(const Instr& instr, SpvType& type)
{
    type.fieldTypes.reserve(instr.numOperands);
    for (std::uint32_t i = 0; i < instr.numOperands; ++i)
    {
        auto fieldType = FindType(instr.GetUInt32(i));
        type.fieldTypes.push_back(fieldType);
        AccumulateSizeInVectorBoundary(type.size, 16, fieldType->size);
    }
    type.size = GetAlignedSize(type.size, 16u);
}

void SpirvReflect::OpTypeOpaque(const Instr& instr, SpvType& type)
{
    //todo
}

void SpirvReflect::OpTypePointer(const Instr& instr, SpvType& type)
{
    type.storage    = static_cast<spv::StorageClass>(instr.GetUInt32(0));
    type.baseType   = FindType(instr.GetUInt32(1));
}

void SpirvReflect::OpTypeFunction(const Instr& instr, SpvType& type)
{
    //todo
}

const SpirvReflect::SpvType* SpirvReflect::FindType(spv::Id id) const
{
    auto it = types_.find(id);
    if (it == types_.end())
        throw std::runtime_error("cannot find SPIR-V OpType* instruction with result ID %" + std::to_string(id));
    return &(it->second);
}

const SpirvReflect::SpvConstant* SpirvReflect::FindConstant(spv::Id id) const
{
    auto it = constants_.find(id);
    if (it == constants_.end())
        throw std::runtime_error("cannot find SPIR-V OpConstant instruction with with result ID %" + std::to_string(id));
    return &(it->second);
}


/*
 * SpvType structure
 */

const SpirvReflect::SpvType* SpirvReflect::SpvType::Deref() const
{
    auto type = this;

    while (type->opcode == spv::Op::OpTypePointer)
    {
        if (type->baseType != nullptr)
            type = type->baseType;
        else
            return nullptr;
    }

    return type;
}

const SpirvReflect::SpvType* SpirvReflect::SpvType::Deref(const spv::Op opcodeType) const
{
    if (auto type = Deref())
        return (type->opcode == opcodeType ? type : nullptr);
    else
        return nullptr;
}

bool SpirvReflect::SpvType::RefersToType(const spv::Op opcodeType) const
{
    return (Deref(opcodeType) != nullptr);
}


} // /namespace LLGL



// ================================================================================
