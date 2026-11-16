# fuzz_banker.py - compara o banker com uma implementacao de referencia (gerado com auxilio de IA)
# Uso (o testar_banker.sh chama sozinho): python3 fuzz_banker.py <executavel> <pasta_temporaria> <quantidade>
import random, subprocess, os, sys
BIN = sys.argv[1]; WORK = sys.argv[2]; N = int(sys.argv[3])
def v(x): return " ".join(map(str, x))
def safe(avail, alloc, need):
    work = avail[:]; fin = [False]*len(alloc); prog = True
    while prog:
        prog = False
        for i in range(len(alloc)):
            if not fin[i] and all(need[i][j] <= work[j] for j in range(len(work))):
                for j in range(len(work)): work[j] += alloc[i][j]
                fin[i] = True; prog = True
    return all(fin)
def ref(avail, mx, cmds):
    m, n = len(mx), len(avail)
    alloc = [[0]*n for _ in range(m)]; need = [r[:] for r in mx]; out = []
    for c in cmds:
        if c[0] == '*':
            out.append("MAXIMUM | ALLOCATION | NEED")
            for i in range(m): out.append(f"{v(mx[i])} | {v(alloc[i])} | {v(need[i])}")
            out.append(f"AVAILABLE {v(avail)}"); continue
        t, i, r = c
        if t == 'RL':
            if any(r[j] > alloc[i][j] for j in range(n)):
                out.append(f"The customer {i} cannot release the resources {v(r)} because it holds only {v(alloc[i])}"); continue
            for j in range(n): alloc[i][j] -= r[j]; need[i][j] += r[j]; avail[j] += r[j]
            out.append(f"Release from customer {i} the resources {v(r)}"); continue
        if any(r[j] > need[i][j] for j in range(n)):
            out.append(f"The customer {i} request {v(r)} was denied because exceed its maximum allocation"); continue
        if any(r[j] > avail[j] for j in range(n)):
            out.append(f"The resources {v(avail)} are not enough to customer {i} request {v(r)}"); continue
        for j in range(n): avail[j] -= r[j]; alloc[i][j] += r[j]; need[i][j] -= r[j]
        if safe(avail, alloc, need):
            out.append(f"Allocate to customer {i} the resources {v(r)}")
        else:
            for j in range(n): avail[j] += r[j]; alloc[i][j] -= r[j]; need[i][j] += r[j]
            out.append(f"The customer {i} request {v(r)} was denied because result in an unsafe state")
    return out
os.chdir(WORK); fails = 0; stats = {}
for k in range(N):
    rnd = random.Random(k)
    n = rnd.randint(1, 5); m = rnd.randint(1, 8)
    avail = [rnd.randint(0, 12) for _ in range(n)]
    mx = [[rnd.randint(0, 10) for _ in range(n)] for _ in range(m)]
    cmds = []
    for _ in range(rnd.randint(1, 30)):
        x = rnd.random()
        if x < 0.1: cmds.append(('*',))
        else:
            i = rnd.randrange(m)
            r = [rnd.randint(0, 4) for _ in range(n)]
            cmds.append(('RL' if x < 0.35 else 'RQ', i, r))
    with open("customer.txt", "w") as f: f.write("".join(",".join(map(str, r)) + "\n" for r in mx))
    with open("commands.txt", "w") as f: f.write("".join(("*" if c[0]=='*' else f"{c[0]} {c[1]} {v(c[2])}") + "\n" for c in cmds))
    exp = ref(avail[:], mx, cmds)
    for line in exp:
        key = line.split()[0] + (" unsafe" if "unsafe" in line else " exceed" if "exceed" in line else " notenough" if "not enough" in line else "")
        stats[key] = stats.get(key, 0) + 1
    if os.path.exists("result.txt"): os.remove("result.txt")
    p = subprocess.run([BIN] + [str(a) for a in avail], capture_output=True, text=True)
    got = open("result.txt").read().splitlines() if os.path.exists("result.txt") else None
    if p.returncode != 0 or got != exp:
        fails += 1
        if fails <= 3:
            print("FALHA no caso", k, "exit", p.returncode, p.stdout[:200])
            for a, b in zip(exp, got or []):
                if a != b: print("  esperado:", a, "\n  obtido:  ", b); break
print(f"{N} casos aleatorios, {fails} falhas")

