import ctypes
import json
from pathlib import Path

from bindings.python.gb_c_definitions import GB, gb

BASE_DIR = Path(__file__).parent.resolve()

SM83_DIR = BASE_DIR / "sm83" / "v1"

# non_cb_tests = sorted(f for f in SM83_DIR.glob("*.json") if not f.name.startswith("cb "))
# cb_tests = sorted(f for f in SM83_DIR.glob("cb *.json"))
all_tests = sorted(f for f in SM83_DIR.glob("*.json"))

def test_ram(ram):
    ok = True
    for addr, val in ram:
        if gb.test_memory_read(addr) != val:
            print(f"\t\tAddress {addr:#X}, actual: {gb.test_memory_read(addr):>3d}, expected: {val:>3d}")
            ok = False
    if not ok:
        print()
    return ok

results = dict()
perfect = 0
# for test_file in non_cb_tests:
for test_file in all_tests:
    correct, wrong = 0, 0
    with open(test_file) as f:
        tests = json.load(f)
    for i, test in enumerate(tests):
        cpu.load_test(test["initial"])
        gb.test_memory_wipe()
        for addr, val in test["initial"]["ram"]:
            gb.test_memory_write(addr, val)
        # gb.instruction_test()
        # gb.fetch_instruction()
        done = False
        cb_done = False
        while not done:
            result = gb.tick_machine_cycle()
            # print(f"{result:#X}")
            # input()
            if ((result >> 8) == 0 and (cb_done or result & 0xFF != 0xCB)):
                done = True
            if result & 0xFF == 0xCB: cb_done = True

        if not cpu.test_final(test["final"]) or not test_ram(test["final"]["ram"]):
            print(f"\t↑↑↑ Test # {i}")
            # print(test)
            wrong += 1
        else: correct += 1
    print(f"{test_file.name}: {correct} / 1000")
    if correct == 1000:
        perfect += 1
    else:
        print(f"↑↑↑ {test["name"]}")
        input()
    # input()
    results[test_file.stem] = correct

print({key: val for key, val in results.items() if val != 1000})
passed_total = sum(results.values())
total = len(all_tests) * 1000
print(f"Total tests: {passed_total} / {total}, {passed_total / total * 100: .2f} %, Instructions passing: {perfect} / {len(all_tests)}")


    # print(tests[0])
    # print()
    # print(tests[0]["initial"])
    # print()


# if __name__ == "__main__":
#     main()