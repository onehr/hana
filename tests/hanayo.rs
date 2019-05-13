extern crate haru;

#[cfg(test)]
pub mod hanayo_tests {

    use haru::ast::grammar;
    use haru::compiler;
    use haru::vm::Vm;
    use haru::vm::VmOpcode;
    use haru::vm::Value;
    use haru::gc;
    use haru::hanayo;

    // TODO
    use haru::vmbindings::carray::CArray;
    use haru::vmbindings::cnativeval::NativeValue;

    macro_rules! eval {
        ($x:expr) => {{
            let prog = grammar::start($x).unwrap();
            let mut c = compiler::Compiler::new();
            gc::set_root(&mut c.vm);
            hanayo::init(&mut c.vm);
            for stmt in prog {
                stmt.emit(&mut c);
            }
            c.vm.code.push(VmOpcode::OP_HALT);
            c.vm.execute();
            c.vm
        }};
    }

    // #region array
    #[test]
    fn array_pop() {
        let mut vm : Vm = eval!("
a = [1,2]
a.pop()
");
        let arr = match vm.global().get("a").unwrap().unwrap() {
            Value::Array(x) => x,
            _ => panic!("expected array")
        };
        assert_eq!(arr.len(), 1);
        assert_eq!(arr[0].unwrap(), Value::Int(1));
    }
    #[test]
    fn array_delete() {
        let mut vm : Vm = eval!("
a = [1,2,3,4,5]
a.delete!(1,2)
");
        let arr = match vm.global().get("a").unwrap().unwrap() {
            Value::Array(x) => x,
            _ => panic!("expected array")
        };
        assert_eq!(arr.len(), 3);
        assert_eq!(arr[0].unwrap(), Value::Int(1));
        assert_eq!(arr[1].unwrap(), Value::Int(4));
        assert_eq!(arr[2].unwrap(), Value::Int(5));
    }
    #[test]
    fn array_index() {
        let mut vm : Vm = eval!("
a = ['a', 'b', 'c']
y = a.index('b')
");
        assert_eq!(vm.global().get("y").unwrap().unwrap(), Value::Int(1));
    }
    #[test]
    fn array_insert() {
        let mut vm : Vm = eval!("
a = [1,2,3]
a.insert!(1, 4)
");
        let arr = match vm.global().get("a").unwrap().unwrap() {
            Value::Array(x) => x,
            _ => panic!("expected array")
        };
        assert_eq!(arr.len(), 4);
        assert_eq!(arr[0].unwrap(), Value::Int(1));
        assert_eq!(arr[1].unwrap(), Value::Int(4));
        assert_eq!(arr[2].unwrap(), Value::Int(2));
        assert_eq!(arr[3].unwrap(), Value::Int(3));
    }
    #[test]
    fn array_map() {
        let mut vm : Vm = eval!("
a=[3,5,64,2]
y = a.map(f(x) = x+1)
");
        let arr = match vm.global().get("y").unwrap().unwrap() {
            Value::Array(x) => x,
            _ => panic!("expected array")
        };
        assert_eq!(arr.len(), 4);
        assert_eq!(arr[0].unwrap(), Value::Int(4));
        assert_eq!(arr[1].unwrap(), Value::Int(6));
        assert_eq!(arr[2].unwrap(), Value::Int(65));
        assert_eq!(arr[3].unwrap(), Value::Int(3));
    }

    #[test]
    fn array_filter() {
        let mut vm : Vm = eval!("
a=[3,5,64,2]
y = a.filter(f(x) = x>5)
");
        let arr = match vm.global().get("y").unwrap().unwrap() {
            Value::Array(x) => x,
            _ => panic!("expected array")
        };
        assert_eq!(arr.len(), 1);
        assert_eq!(arr[0].unwrap(), Value::Int(64));
    }

    #[test]
    fn array_reduce() {
        let mut vm : Vm = eval!("
a=[1,2,3,4,5]
y = a.reduce(f(x, y) = x + y, 0)
");
        assert_eq!(vm.global().get("y").unwrap().unwrap(), Value::Int(15));
    }

    #[test]
    fn array_chained_functional() {
        let mut vm : Vm = eval!("
a=[1,2,3,5,6]
y = a.map(f(x) = x+1).filter(f(x) = x>5).reduce(f(prev, curr) = prev+curr, 0)
");
        assert_eq!(vm.global().get("y").unwrap().unwrap(), Value::Int(13));
    }
    // #endregion

}