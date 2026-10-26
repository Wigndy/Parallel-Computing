# 1. Biên dịch dự án (Build)

```bash
cmake -B build -S .
cmake --build build -j
```

---

# 2. Kiểm tra tính đúng đắn (Test Correctness)
*Số lần chạy:* **1 lần**

```bash
./build/tests/test_locks
```

---

# 3. Bài toán 1: Mutex Locks Benchmark (`is_prime`)
*Số lần chạy mỗi lệnh:* **10 lần** (để lấy giá trị trung bình thống kê)

### 3.1. Khảo sát tranh chấp (Contention Sweep - Threads = 4, Iters = 5000)
```bash
# 100% Critical Section (Tranh chấp cực cao)
./build/bin/bench_prime_locks --threads 4 --iters 5000 --outside-work 0 --lock all

# 50% Critical Section (Chuẩn đề bài)
./build/bin/bench_prime_locks --threads 4 --iters 5000 --outside-work 1 --lock all

# 17% Critical Section (Tranh chấp vừa)
./build/bin/bench_prime_locks --threads 4 --iters 5000 --outside-work 5 --lock all

# 5% Critical Section (Tranh chấp thấp)
./build/bin/bench_prime_locks --threads 4 --iters 5000 --outside-work 20 --lock all
```

### 3.2. Khảo sát khả năng mở rộng (Scalability Sweep)
```bash
for t in 1 2 4 8 16; do
    for ow in 0 1 5; do
        ./build/bin/bench_prime_locks --threads $t --iters 5000 --outside-work $ow --lock all
    done
done
```

---

# 4. Bài toán 2: Realtime Banking & Deadlock Avoidance Benchmark
*Số lần chạy mỗi lệnh:* **10 lần**

### 4.1. Khảo sát tỷ lệ Đọc/Ghi (Read-Ratio Sweep - Threads = 8, Accounts = 50, Ops = 10000)
```bash
# 99% Read, 1% Write (Read-heavy)
./build/bin/bench_banking_locks --threads 8 --accounts 50 --ops 10000 --timeout-us 1000 --read-ratio 0.99

# 80% Read, 20% Write
./build/bin/bench_banking_locks --threads 8 --accounts 50 --ops 10000 --timeout-us 1000 --read-ratio 0.80

# 50% Read, 50% Write (Cân bằng)
./build/bin/bench_banking_locks --threads 8 --accounts 50 --ops 10000 --timeout-us 1000 --read-ratio 0.50

# 10% Read, 90% Write (Write-heavy)
./build/bin/bench_banking_locks --threads 8 --accounts 50 --ops 10000 --timeout-us 1000 --read-ratio 0.10
```

### 4.2. Thử nghiệm triệt tiêu Deadlock (Deadlock Torture Test)
```bash
./build/bin/bench_banking_locks --threads 16 --accounts 2 --ops 10000 --timeout-us 100 --read-ratio 0.0
```

### 4.3. Khảo sát độ nhạy Timeout (Timeout Sweep - Threads = 8, Accounts = 10, Ops = 5000)
```bash
# Timeout 50 us
./build/bin/bench_banking_locks --threads 8 --accounts 10 --ops 5000 --timeout-us 50 --read-ratio 0.2

# Timeout 200 us
./build/bin/bench_banking_locks --threads 8 --accounts 10 --ops 5000 --timeout-us 200 --read-ratio 0.2

# Timeout 1000 us (1 ms)
./build/bin/bench_banking_locks --threads 8 --accounts 10 --ops 5000 --timeout-us 1000 --read-ratio 0.2

# Timeout 5000 us (5 ms)
./build/bin/bench_banking_locks --threads 8 --accounts 10 --ops 5000 --timeout-us 5000 --read-ratio 0.2
```