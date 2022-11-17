#include "RE/Bethesda/BSScript.h"

#include "RE/Bethesda/BSScript.h"
#include "RE/Bethesda/GameScript.h"

namespace RE::BSScript
{
	auto TypeInfo::GetRawType() const
		-> RawType
	{
		if (IsComplex()) {
			const auto complex =
				reinterpret_cast<IComplexType*>(
					reinterpret_cast<std::uintptr_t>(data.complexTypeInfo) &
					~static_cast<std::uintptr_t>(1));
			return complex->GetRawType();
		} else {
			return *data.rawType;
		}
	}

	Variable& Variable::operator=(BSTSmartPointer<Object> a_object)
	{
		reset();
		if (a_object) {
			value.o = std::move(a_object);
			varType = value.o->type.get();
		} else {
			assert(false);
		}

		assert(is<Object>());
		return *this;
	}

	Variable& Variable::operator=(BSTSmartPointer<Struct> a_struct)
	{
		reset();
		if (a_struct) {
			value.t = std::move(a_struct);
			varType = value.t->type.get();
		} else {
			assert(false);
		}

		assert(is<Struct>());
		return *this;
	}

	Variable& Variable::operator=(BSTSmartPointer<Array> a_array)
	{
		reset();
		if (a_array) {
			value.a = std::move(a_array);
			varType = value.a->elementType;
			varType.SetArray(true);
		} else {
			assert(false);
		}

		assert(is<Array>());
		return *this;
	}

	void Variable::reset()
	{
		switch (varType.GetRawType()) {
		case RawType::kObject:
			value.o.reset();
			break;
		case RawType::kString:
			value.s.~BSFixedString();
			break;
		case RawType::kVar:
			delete value.v;
			break;
		case RawType::kStruct:
			value.t.reset();
			break;
		case RawType::kArrayObject:
		case RawType::kArrayString:
		case RawType::kArrayInt:
		case RawType::kArrayFloat:
		case RawType::kArrayBool:
		case RawType::kArrayVar:
		case RawType::kArrayStruct:
			value.a.reset();
			break;
		case RawType::kNone:
		case RawType::kInt:
		case RawType::kFloat:
		case RawType::kBool:
			break;
		default:
			assert(false);  // unhandled type
			break;
		}

		varType = RawType::kNone;
		value.v = nullptr;
		assert(is<std::nullptr_t>());
	}

	// NOLINTNEXTLINE(misc-no-recursion)
	void Variable::copy(const Variable& a_rhs)
	{
		assert(varType.GetRawType() == RawType::kNone);
		assert(value.v == nullptr);

		switch (a_rhs.varType.GetRawType()) {
		case RawType::kObject:
			value.o = a_rhs.value.o;
			break;
		case RawType::kString:
			value.s = a_rhs.value.s;
			break;
		case RawType::kVar:
			if (a_rhs.value.v) {
				value.v = new Variable(*a_rhs.value.v);
			}
			break;
		case RawType::kStruct:
			value.t = a_rhs.value.t;
			break;
		case RawType::kArrayObject:
		case RawType::kArrayString:
		case RawType::kArrayInt:
		case RawType::kArrayFloat:
		case RawType::kArrayBool:
		case RawType::kArrayVar:
		case RawType::kArrayStruct:
			value.a = a_rhs.value.a;
			break;
		case RawType::kNone:
			break;
		case RawType::kInt:
			value.u = a_rhs.value.u;
			break;
		case RawType::kFloat:
			value.f = a_rhs.value.f;
			break;
		case RawType::kBool:
			value.b = a_rhs.value.b;
			break;
		default:
			assert(false);  // unhandled type
			break;
		}

		varType = a_rhs.varType;
	}

	Object::~Object()
	{
		if (Constructed()) {
			const std::uint32_t size = type ? type->GetVariableCount() : 0;
			for (std::uint32_t i = 0; i < size; ++i) {
				variables[i].reset();
			}

			constructed = false;
			initialized = false;
		}

		auto lock = reinterpret_cast<std::uintptr_t>(lockStructure) & ~static_cast<std::uintptr_t>(1);
		if (lock) {
			stl::atomic_ref l{ lock };
			--l;
		}
	}

	Struct::~Struct()
	{
		if (constructed) {
			const std::uint32_t size = type ? type->variables.size() : 0;
			for (std::uint32_t i = 0; i < size; ++i) {
				variables[i].reset();
			}

			constructed = false;
		}
	}
}
