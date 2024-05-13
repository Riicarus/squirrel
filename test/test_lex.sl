{
    // int k = ;
    // func test() true {};
    // func test(int k; int p) int {};
    // int k = 10,

    int i = 0;
    float f = -0.0;
    bool on = true;
    on = false;
    char c = 'c';
    string s = "str";
    i = 3 + 5;

    // func, if
    func comp(int a, int b) int {
        int k = i + 9;
        if (a > b) { return i; }
        elseif (a == k) { return 0; }
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

    func test_bool(bool x) bool {
        bool y = !x;
        bool z = ~x;
        return y & z;
    };

    func void_func() void {
        return;
    };

    int k = comp(1, 2);
    k = recursive_add(comp(4, 6));

    void_func();
}