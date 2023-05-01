fun add(a : int, b : int) : int {
    ret a + b;
}

fun main() : int {
    let a : int = 1;
    let b : int = 2;
    let c : int = add(a, b);
    dbg a;
    dbg b;
    dbg c;
    ret 0;
}