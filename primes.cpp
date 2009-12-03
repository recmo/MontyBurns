#include <primes.h>
#include <stdlib.h>

const uint64s small_primes = prime_sieve(1<<20);

uint64s prime_sieve(uint64 max)
{
	vector<bool> sieve((max+1)/2);
	uint64s primes;
	primes.push_back(2);
	for(uint64 i=3; i <= max; i+=2)
	{
		if(sieve[(i-3)/2]) continue;
		for(uint64 m=i*i; m <= max; m+=2*i)
			sieve[(m-3)/2] = true;
		primes.push_back(i);
	}
	return primes;
}

uint64 mul(uint64 a, uint64 b, uint64 m)
{
	// Perform 128 multiplication and division
	uint64 q; // q = ⌊a b / m⌋
	uint64 r; // r = a b mod m
	asm(
		"mulq %3;"
		"divq %4;"
		: "=a"(q), "=d"(r)
		: "a"(a), "rm"(b), "rm"(m));
	return r;
}

uint64 pow(uint64 b, uint64 e, uint64 m)
{
	uint64 r = 1;
	for(; e; e >>= 1)
	{
		if(e & 1) r = mul(r, b, m);
		b = mul(b, b, m);
	}
	return r;
}


/// Miller-Rabin probabilistic primality testing
/// @param n The number to test for  primality
/// @param k The witness for primality
/// @returns True iff when n is a k-stong pseudoprime
bool miller_rabin(uint64 n, uint64 k)
{
	// Factor n-1 as d*2^s
	uint64 s = 0;
	uint64 d = n - 1;
	for(; !(d & 1); s++)
		d >>= 1;

	// Verify x = k^(d 2^i) mod n != 1
	uint64 x = pow(k % n, d, n);
	if(x == 1 || x == n-1) return true;
	while(s-- > 1)
	{
		// x = x^2 mod n
		x = mul(x, x, n);
		if(x == 1) return false;
		if(x == n-1) return true;
	}
	return false;
}

/// Miller-Rabin probabilistic primality testing
/// @param n The number to test for  primality
/// @returns False when n is not a prime
bool is_prime(uint64 n)
{
	// Handle small primes fast
	for(int i = 0; i < 100; i++)
	{
		uint64 p = small_primes[i];
		if(n == p) return true;
		if(n % p == 0) return false;
	}
	
	// Do a few Miller-Rabin rounds
	for(int i = 0; i < 10; i++)
	{
		if (!miller_rabin(n, small_primes[i])) return false;
	}
	return true;
}


// Source: http://en.wikipedia.org/wiki/Binary_GCD_algorithm
uint64 gcd(uint64 u, uint64 v)
{
	int shift;
	
	// GCD(0,x) := x
	if (u == 0 || v == 0)
		return u | v;
	
	/* Let shift := lg K, where K is the greatest power of 2
	dividing both u and v. */
	for (shift = 0; ((u | v) & 1) == 0; ++shift)
	{
		u >>= 1;
		v >>= 1;
	}
	
	while ((u & 1) == 0)
		u >>= 1;
	
	/* From here on, u is always odd. */
	do {
		while ((v & 1) == 0)  /* Loop X */
			v >>= 1;
		
		/* Now u and v are both odd, so diff(u, v) is even.
		Let u = min(u, v), v = diff(u, v)/2. */
		if (u < v) {
			v -= u;
		} else {
			uint64 diff = u - v;
			u = v;
			v = diff;
		}
		v >>= 1;
	} while (v != 0);
	
	return u << shift;
}

// Source: http://www.frenchfries.net/paul/factoring/source.html
// Converted to uint64
uint64 pollard_rho_factor(uint64 n)
{
	int i = 1;
	int j = 1;
	
	uint64 a = (random() & 32) - 16;
	uint64 x = random() & 31;
	uint64 q = 1;
	uint64 y = x;
	
	for(;;)
	{
		// TODO: Check overflows!
		x  = (x*x + a)  %  n;
		y  = (y*y + a)  %  n;
		y  = (y*y + a)  %  n;
		q *= (x - y);
		q %= n;
		
		i++;
		if(!j) j = 1;
		
		if((i % j) == 0)
		{
			j++;
			uint64 d = gcd(q, n);
			if (d != 1)
			{
				if (!is_prime(d))
					return pollard_rho_factor(d);
				else return d;
			}
		} // if ( (i % j) == 0)
	} // while n != 1
	return 0;
}

vector<uint64> prime_factors(uint64 n)
{
	vector<uint64> f;
	
	// Remove the twos
	if((n & 1) == 0)
	{
		f.push_back(2);
		do n >>= 1; while((n & 1) == 0);
	}
	
	// Remove other factors
	for(int i=3; i * i < n ; i += 2)
	{
		if(n % i == 0)
		{
			f.push_back(i);
			do n /= i; while(n % i == 0);
		}
	}
	f.push_back(n);
	return f;
}
