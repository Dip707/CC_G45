fun not(a: int) : int {
    if a {
        ret 0;
    } else {
        ret 1;
    }
}

fun main(): int {
    dbg not(0);
}