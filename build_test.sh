rm -rf build
cmake -B build -DNEED_TEST=ON
cmake --build build
bin/squirrel_test

mem_check="false"

for arg in "$@"; do
    case $arg in
        --mem-check)
            mem_check="true"
            ;;
        *)
            # other args or error msg
            echo "unknown option: $arg"
            ;;
    esac
done

if [ "$mem_check" = "true" ]; then
    #valgrind --log-file=./bin/valgrind.log --tool=memcheck --leak-check=full --show-leak-kinds=all ./bin/squirrel_test
    valgrind --log-file=./bin/valgrind.log --tool=memcheck ./bin/squirrel_test
fi