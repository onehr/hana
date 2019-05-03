mod ast;
use std::io::Read;

fn main() {
    let args : Vec<String> = std::env::args().collect();
    let mut file = match std::fs::File::open(&args[1]) {
        Err(e) => { panic!("Error opening file: {}", e); }
        Ok(f) => f
    };
    let mut s = String::new();
    match file.read_to_string(&mut s) {
        Err(e) => { panic!("Error reading file: {}", e); }
        Ok(_) => { }
    };
    match ast::grammar::start(&s) {
        Ok(r) => println!("{:?}", r),
        Err(e) => println!("Parsed error: {}", e)
    };
}