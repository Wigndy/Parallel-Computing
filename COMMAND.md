# 1. Biên dịch dự án (Build)

```bash
cmake -B build -S .
cmake --build build -j
```

---

# 2. Kiểm tra tính đúng đắn (Test Correctness)

```bash
./build/tests/test_locks
```

---

# 3. Bài toán 1: Mutex Locks Benchmark (`is_prime`)

### 3.1. Khảo sát tranh chấp (Contention Sweep - Threads = 4, Iters = 5000)

**Chạy toàn bộ tự động 10 lần mỗi mức tranh chấp:**
```bash
for ow in 0 1 5 20; do
    echo "=== OUTSIDE_WORK = $ow (10 runs) ==="
    for r in {1..10}; do
        ./build/bin/bench_prime_locks --threads 4 --iters 5000 --outside-work $ow --lock all
    done
done
```

**Hoặc chạy lẻ từng kịch bản (tự động lặp 10 lần):**
```bash
# 100% Critical Section (Tranh chấp cực cao) - 10 lần
for i in {1..10}; do ./build/bin/bench_prime_locks --threads 4 --iters 5000 --outside-work 0 --lock all; done

# 50% Critical Section (Chuẩn đề bài) - 10 lần
for i in {1..10}; do ./build/bin/bench_prime_locks --threads 4 --iters 5000 --outside-work 1 --lock all; done

# 17% Critical Section (Tranh chấp vừa) - 10 lần
for i in {1..10}; do ./build/bin/bench_prime_locks --threads 4 --iters 5000 --outside-work 5 --lock all; done

# 5% Critical Section (Tranh chấp thấp) - 10 lần
for i in {1..10}; do ./build/bin/bench_prime_locks --threads 4 --iters 5000 --outside-work 20 --lock all; done
```

### 3.2. Khảo sát khả năng mở rộng (Scalability Sweep - Tự động chạy 10 lần mỗi cấu hình)
```bash
for t in 1 2 4 8 16; do
    for ow in 0 1 5; do
        echo "=== Threads: $t | OutsideWork: $ow (10 runs) ==="
        for r in {1..10}; do
            ./build/bin/bench_prime_locks --threads $t --iters 5000 --outside-work $ow --lock all
        done
    done
done
```

---

# 4. Bài toán 2: Realtime Banking & Deadlock Avoidance Benchmark

### 4.1. Khảo sát tỷ lệ Đọc/Ghi (Read-Ratio Sweep - Threads = 8, Accounts = 50, Ops = 10000)

**Chạy toàn bộ tự động 10 lần mỗi tỷ lệ:**
```bash
for rr in 0.99 0.80 0.50 0.10; do
    echo "=== Read Ratio: $rr (10 runs) ==="
    for r in {1..10}; do
        ./build/bin/bench_banking_locks --threads 8 --accounts 50 --ops 10000 --timeout-us 1000 --read-ratio $rr
    done
done
```

**Hoặc chạy lẻ từng tỷ lệ (tự động lặp 10 lần):**
```bash
# 99% Read, 1% Write (Read-heavy) - 10 lần
for i in {1..10}; do ./build/bin/bench_banking_locks --threads 8 --accounts 50 --ops 10000 --timeout-us 1000 --read-ratio 0.99; done

# 80% Read, 20% Write - 10 lần
for i in {1..10}; do ./build/bin/bench_banking_locks --threads 8 --accounts 50 --ops 10000 --timeout-us 1000 --read-ratio 0.80; done

# 50% Read, 50% Write (Cân bằng) - 10 lần
for i in {1..10}; do ./build/bin/bench_banking_locks --threads 8 --accounts 50 --ops 10000 --timeout-us 1000 --read-ratio 0.50; done

# 10% Read, 90% Write (Write-heavy) - 10 lần
for i in {1..10}; do ./build/bin/bench_banking_locks --threads 8 --accounts 50 --ops 10000 --timeout-us 1000 --read-ratio 0.10; done
```

### 4.2. Thử nghiệm triệt tiêu Deadlock (Deadlock Torture Test - Tự động lặp 10 lần)
```bash
for i in {1..10}; do ./build/bin/bench_banking_locks --threads 16 --accounts 2 --ops 10000 --timeout-us 100 --read-ratio 0.0; done
```

### 4.3. Khảo sát độ nhạy Timeout (Timeout Sweep - Threads = 8, Accounts = 10, Ops = 5000)

**Chạy toàn bộ tự động 10 lần mỗi mốc timeout:**
```bash
for to in 50 200 1000 5000; do
    echo "=== Timeout: ${to}us (10 runs) ==="
    for r in {1..10}; do
        ./build/bin/bench_banking_locks --threads 8 --accounts 10 --ops 5000 --timeout-us $to --read-ratio 0.2
    done
done
```

**Hoặc chạy lẻ từng mốc timeout (tự động lặp 10 lần):**
```bash
# Timeout 50 us - 10 lần
for i in {1..10}; do ./build/bin/bench_banking_locks --threads 8 --accounts 10 --ops 5000 --timeout-us 50 --read-ratio 0.2; done

# Timeout 200 us - 10 lần
for i in {1..10}; do ./build/bin/bench_banking_locks --threads 8 --accounts 10 --ops 5000 --timeout-us 200 --read-ratio 0.2; done

# Timeout 1000 us (1 ms) - 10 lần
for i in {1..10}; do ./build/bin/bench_banking_locks --threads 8 --accounts 10 --ops 5000 --timeout-us 1000 --read-ratio 0.2; done

# Timeout 5000 us (5 ms) - 10 lần
for i in {1..10}; do ./build/bin/bench_banking_locks --threads 8 --accounts 10 --ops 5000 --timeout-us 5000 --read-ratio 0.2; done
```