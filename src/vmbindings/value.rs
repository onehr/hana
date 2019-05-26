//! Provides an abstraction for native values

use super::carray::CArray;
use super::cnativeval::{NativeValue, _valueType};
use super::function::Function;
use super::gc::Gc;
use super::record::Record;
use super::vm::Vm;
extern crate libc;

pub type NativeFnData = extern "C" fn(*mut Vm, u16);

#[derive(Clone)]
#[allow(non_camel_case_types, dead_code)]
/// Wrapper for native values
pub enum Value {
    // we don't have control over how rust manages its variant
    // types, so this is a convenient wrapper for (de)serialising
    // hana's values
    Nil,
    True,
    False,

    Int(i64),
    Float(f64),
    NativeFn(NativeFnData),
    Fn(Gc<Function>),
    Str(Gc<String>),
    Record(Gc<Record>),
    Array(Gc<CArray<NativeValue>>),
}

#[allow(improper_ctypes)]
extern "C" {
    fn value_get_prototype(vm: *const Vm, val: NativeValue) -> *const Record;
    fn value_is_true(left: NativeValue, vm: *const Vm) -> bool;
}

impl Value {
    // #region coerce value to type
    #[cfg_attr(tarpaulin, skip)]
    pub fn int(&self) -> i64 {
        match self {
            Value::Int(s) => *s,
            _ => {
                panic!("Expected integer");
            }
        }
    }

    #[cfg_attr(tarpaulin, skip)]
    pub fn float(&self) -> f64 {
        match self {
            Value::Float(s) => *s,
            _ => {
                panic!("Expected float");
            }
        }
    }

    #[cfg_attr(tarpaulin, skip)]
    pub fn string(&self) -> &'static String {
        match self {
            Value::Str(s) => unsafe { &*s.to_raw() },
            _ => {
                panic!("Expected string");
            }
        }
    }

    #[cfg_attr(tarpaulin, skip)]
    pub fn array(&self) -> &'static CArray<NativeValue> {
        match self {
            Value::Array(s) => unsafe { &*s.to_raw() },
            _ => {
                panic!("Expected array");
            }
        }
    }

    #[cfg_attr(tarpaulin, skip)]
    pub fn record(&self) -> &'static Record {
        match self {
            Value::Record(rec) => unsafe { &*rec.to_raw() },
            _ => {
                panic!("Expected record");
            }
        }
    }
    // #endregion

    // wrapper for native
    pub fn wrap(&self) -> NativeValue {
        use std::mem::transmute;
        #[allow(non_camel_case_types)]
        unsafe {
            match &self {
                Value::Nil => NativeValue {
                    r#type: _valueType::TYPE_NIL,
                    data: 0,
                },
                Value::True => NativeValue {
                    r#type: _valueType::TYPE_INT,
                    data: 1,
                },
                Value::False => NativeValue {
                    r#type: _valueType::TYPE_INT,
                    data: 0,
                },
                Value::Int(n) => NativeValue {
                    r#type: _valueType::TYPE_INT,
                    data: transmute::<i64, u64>(*n),
                },
                Value::Float(n) => NativeValue {
                    r#type: _valueType::TYPE_FLOAT,
                    data: transmute::<f64, u64>(*n),
                },
                Value::NativeFn(f) => NativeValue {
                    r#type: _valueType::TYPE_NATIVE_FN,
                    data: transmute::<NativeFnData, u64>(*f),
                },
                Value::Fn(p) => NativeValue {
                    r#type: _valueType::TYPE_FN,
                    data: transmute::<*const Function, u64>(p.to_raw()),
                },
                Value::Str(p) => NativeValue {
                    r#type: _valueType::TYPE_STR,
                    data: transmute::<*const String, u64>(p.to_raw()),
                },
                Value::Record(p) => NativeValue {
                    r#type: _valueType::TYPE_DICT,
                    data: transmute::<*const Record, u64>(p.to_raw()),
                },
                Value::Array(p) => NativeValue {
                    r#type: _valueType::TYPE_ARRAY,
                    data: transmute::<*const CArray<NativeValue>, u64>(p.to_raw()),
                },
                _ => unimplemented!(),
            }
        }
    }

    // prototype
    pub fn get_prototype(&self, vm: *const Vm) -> *const Record {
        unsafe { value_get_prototype(vm, self.wrap()) }
    }

    // bool
    pub fn is_true(&self, vm: *const Vm) -> bool {
        unsafe { value_is_true(self.wrap(), vm) }
    }
}

use std::cmp::PartialEq;
/// Exact equality for values. This is an internal trait for tests.
///
/// Note that equality in Value structs aren't necessarily the same
/// in the language.
impl PartialEq for Value {
    #[cfg_attr(tarpaulin, skip)]
    fn eq(&self, other: &Value) -> bool {
        match (self, other) {
            (Value::Nil, Value::Nil) => true,
            (Value::Int(x), Value::Int(y)) => x == y,
            (Value::Float(x), Value::Float(y)) => x == y,
            (Value::NativeFn(x), Value::NativeFn(y)) => std::ptr::eq(x, y),
            (Value::Fn(x), Value::Fn(y)) => x.ptr_eq(y),
            (Value::Str(x), Value::Str(y)) => x.as_ref() == y.as_ref(),
            (Value::Record(_), Value::Record(_)) => false,
            (Value::Array(_), Value::Array(_)) => false,
            _ => false,
        }
    }
}

use std::fmt;

#[cfg_attr(tarpaulin, skip)]
impl fmt::Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Value::Nil => write!(f, "[nil]"),
            Value::True => write!(f, "1"),
            Value::False => write!(f, "0"),
            Value::Int(n) => write!(f, "{}", n),
            Value::Float(n) => write!(f, "{}", n),
            Value::NativeFn(_) => write!(f, "[native fn]"),
            Value::Fn(_) => write!(f, "[fn]"),
            Value::Str(p) => write!(f, "{}", p.as_ref()),
            Value::Record(p) => write!(f, "[record {:p}]", p.to_raw()),
            Value::Array(p) => write!(f, "[array {:p}]", p.to_raw()),
        }
    }
}

#[cfg_attr(tarpaulin, skip)]
impl fmt::Debug for Value {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Value::Nil => write!(f, "[nil]"),
            Value::Int(n) => write!(f, "{}", n),
            Value::Float(n) => write!(f, "{}", n),
            Value::NativeFn(_) => write!(f, "[native fn]"),
            Value::Fn(_) => write!(f, "[fn]"),
            Value::Str(p) => {
                let mut s = String::new();
                for ch in p.as_ref().chars() {
                    match ch {
                        '\n' => s.push_str("\\n"),
                        '"' => s.push('"'),
                        _ => s.push(ch)
                    }
                }
                write!(f, "\"{}\"", s)
            }
            Value::Record(p) => write!(f, "[record {:p}]", p.to_raw()),
            Value::Array(p) => write!(f, "[array {:p}]", p.to_raw()),
            _ => write!(f, "[unk]"),
        }
    }
}
