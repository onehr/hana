use super::env::Env;
use std::ptr::null_mut;
use crate::vmbindings::gc::GcTraceable;

// functions
#[repr(C)]
#[derive(Clone)]
pub struct Function {
    pub ip: u32, // instruction pointer
    pub nargs : u16, // number of args

    // internal rust properties:
    pub bound: Env,
    // represents the current local environment
    // at the time the function is declared, this will be
    // COPIED into another struct env whenever OP_CALL is issued
    // (we use this to implement closures)
}

impl Function {

    pub unsafe fn new(ip: u32, nargs: u16, env: *const Env) -> Function {
        Function {
            ip: ip,
            nargs: nargs,
            bound: if env.is_null() { Env::new(0, null_mut(), nargs) }
                   else { Env::copy(&*env) }
        }
    }

    pub unsafe fn get_bound_ptr(&self) -> *const Env {
        &self.bound
    }

}

// gc traceable
impl GcTraceable for Function {

    fn trace(ptr: *mut libc::c_void) {
        unsafe {
            let self_ = &*(ptr as *mut Self);
            for val in self_.bound.slots.iter() {
                val.trace();
            }
        }
    }

}