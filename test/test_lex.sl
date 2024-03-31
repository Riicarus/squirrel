{
    // hello
    // basic types
    int i = 0;
    float f = -0.0;
    // char i = 'i';
    bool on = true;
    on = false;
    char c = 'c';
    string s = "str";

    // return;
    // break;

    // functions & controls
    // func, if
    func comp(int a, int b) int {
        if (a > b) { return 1; }
        elseif (a == b) { return 0; }
        else { return -1; };

        return 100;
    };

    // func, for
    func recursive_add(int n) int {
        int sum = 0;
        for (int i = 0; i <= n; i ++) {
            sum = sum + i;
            sum++;
            --sum;
        };
        // return sum + "1";
        return sum;
    };

    // other symbols
    func test_other_symbols() int {
        int i = -1;
        i = (2 << 1) * i;
        if (i != 1) { return 0; }
        elseif (i == 3) { return 2; }
        else { return -1; };
        return 1;
    };

    int k = comp(1, 2);
    k = recursive_add(comp(4, 6));

    for (int i = 0; i < 10; i++) {
        if (i == 1) { return; };
        i = i + 2;
    };

    func void_func() void {
        return;
    };

    void_func();
}