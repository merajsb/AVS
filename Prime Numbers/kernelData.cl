__kernel void sieveofAktin(const int limit, __global bool* sieve)
{
    int x = get_global_id(0);
    for (int y = 1; y * y < limit; y++) {
        int n = (4 * x * x) + (y * y);
        if (n <= limit && (n % 12 == 1 || n % 12 == 5))
            sieve[n] ^= true;

        n = (3 * x * x) + (y * y);
        if (n <= limit && n % 12 == 7)
            sieve[n] ^= true;

        n = (3 * x * x) - (y * y);
        if (x > y && n <= limit && n % 12 == 11)
            sieve[n] ^= true;
    }
    for (int r = 5; r * r < limit; r++) {
        if (sieve[r]) {
            for (int i = r * r; i < limit; i += r * r)
                sieve[i] = false;
        }
    }

}