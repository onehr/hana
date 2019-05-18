use super::env::Env;
use std::ptr::null_mut;
use std::mem::ManuallyDrop;

// functions
#[repr(C)]
#[derive(Clone)]
pub struct Function {
    pub ip: u32, // instruction pointer
    pub nargs : u16, // number of args

    // internal rust properties:
    pub bound: ManuallyDrop<Env>,
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
            bound: ManuallyDrop::new(
                    if env.is_null() { Env::new(0, null_mut(), nargs) }
                    else { Env::copy(&*env) }
                )
        }
    }

    pub fn drop(&mut self) { // must be called by the gc
        unsafe{ ManuallyDrop::drop(&mut self.bound); }
    }

    pub unsafe fn get_bound_ptr(&mut self) -> *mut Env {
        &mut *self.bound
    }

    pub fn mark(&self) {
        for i in 0..self.bound.nslots {
            unsafe { self.bound.get(i) }.mark();
        }
    }

    pub fn pin(&self) {
        for i in 0..self.bound.nslots {
            unsafe{ self.bound.get(i) }.pin();
        }
    }

}