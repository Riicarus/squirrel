{
    int i = 0;
    float f = -0.0;
    bool on = true;
    on = false;
    char c = 'c';
    string s = "str";
    i = 3 + 5;

    i = comp(i, 2);

    func comp(int a, int b) int {
        int k = i + 9;
        if (a > b) { return i; }
        elseif (a == k) { return 0; }
        else { 
            i = comp(3, 4);
            return -1; 
        };

        return 100;
    };

    func unused() void {
        int k = 0;
    };

    i = comp(i, 2);
    int k = comp(i, 3);
}

