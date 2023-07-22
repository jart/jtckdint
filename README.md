# C23 Checked Arithmetic

[jtckdint.h](jtckdint.h) is a portable single-file header-only library
that defines three type generic functions:

- `bool ckd_add(res, a, b)`
- `bool ckd_sub(res, a, b)`
- `bool ckd_mul(res, a, b)`

Which allow integer arithmetic errors to be detected. There are many
kinds of integer errors, e.g. overflow, truncation, etc. These funcs
catch them all. Here's an example of how it works:

```c
#include "jtckdint.h"
uint32_t c;
int32_t a = 0x7fffffff;
int32_t b = 2;
assert(!ckd_add(&c, a, b));
assert(c == 0x80000001u);
```

Experienced C / C++ users should find this example counter-intuitive
because the expression `0x7fffffff + 2` not only overflows it's also
undefined behavior. However here we see it's specified, and does not
result in an error. That's because C23 checked arithmetic is not the
arithmetic you're used to.

C23 checked arithmetic is defined as performing the arithmetic using
infinite precision and then checking if the resulting value will fit
in the output type. Our example above did not result in an error due
to `0x80000001` being a legal value for `uint32_t`.

This implementation will use the GNU compiler builtins, when they're
available, only if you don't use build flags like `-std=c11` because
they define `__STRICT_ANSI__` and GCC extensions aren't really ANSI.
Instead, you'll get a pretty good pure C11 and C++11 implementation.

## Correctness

Most everyone who's implemented C23 checked arithmetic has gotten it
wrong. Even GNU and Intel have shipped incorrect implementations. We
think we got it right. In [test.c](test.c) you'll find comprehensive
tests of all type combinations which should demonstrate the behavior
of our polyfills is consistent with the GCC/Clang compiler builtins.
You may also run `./test.sh` to test lots of build modes e.g. UBSAN.

Part of what makes this complicated, is there's a thousand different
possible type combinations. Even when the language has generics that
isn't easy. We make it easy by just promoting everything to intmax_t
which means we only have to perform clever bit hacks when one of the
types is the maximum type. Those hacks are:

|       | z = x + y                          | z = x - y                          | z = x * y                                    |
|-------|------------------------------------|------------------------------------|----------------------------------------------|
| u=u∙u | z < x                              | x < y                              | x && z / x != y                              |
| u=u∙s | ((z ^ x) & (z ^ y ^ min)) < 0      | ((x ^ y ^ min) & (z ^ x)) < 0      | x && z / x != y \|\| y < 0 && x              |
| u=s∙u | ((z ^ x ^ min) & (z ^ y)) < 0      | y > x \| x < 0                     | x && z / x != y \|\| x < 0 && y              |
| u=s∙s | ((z \| x) & y \| (z & x) & ~y) < 0 | ((z \| x) & ~y \| (z & x) & y) < 0 | hoo boy                                      |
| s=u∙u | z < x \| z < 0                     | x < y ^ z < 0                      | x && z / x != y \|\| z < 0                   |
| s=u∙s | y ^= min, x + y < y                | x >= (y ^ min)                     | hoo boy                                      |
| s=s∙u | x ^= min, x + y < x                | (x ^ min) < y                      | hoo boy                                      |
| s=s∙s | ((z ^ x) & (z ^ y)) < 0            | ((x ^ y) & (z ^ x)) < 0            | y < 0 && x == min \|\| y && (s)z / (s)y != x |

These are the best bit hacks I've found, for detecting overflow with
mixed-signedness expressions. The first row is what what you'll find
online with Google. The last row is also commonly known, and easy to
find in books like Hacker's Delight. In the middle is our greenfield
where I've even added my own personal touch.

## Quality

The C11 and C++11 polyfills provided by jtckdint.h generate fabulous
machine code which is in some cases actually better than the builtin
functions provided by your compiler! Here's the output of `demo.sh`,
which can help you compare the quality.

```
========================================================================
jtckdint.h using compiler builtins
cc -S -Os -o - demo.c
========================================================================


ckd_add_unsigned_unsigned_unsigned:
	addq	%rdx, %rsi
	movq	%rsi, (%rdi)
	setc	%al
	ret

ckd_add_signed_signed_signed:
	addq	%rdx, %rsi
	movq	%rsi, (%rdi)
	seto	%al
	ret

ckd_add_unsigned_signed_signed:
	xorl	%eax, %eax
	leaq	(%rsi,%rdx), %rcx
	testq	%rdx, %rdx
	js	.L10
	testq	%rcx, %rsi
	jmp	.L12
.L10:
	orq	%rcx, %rsi
.L12:
	jns	.L8
	movl	$1, %eax
.L8:
	movq	%rcx, (%rdi)
	ret

ckd_sub_unsigned_unsigned_unsigned:
	subq	%rdx, %rsi
	movq	%rsi, (%rdi)
	setb	%al
	ret

ckd_sub_signed_signed_signed:
	subq	%rdx, %rsi
	movq	%rsi, (%rdi)
	seto	%al
	ret

ckd_sub_unsigned_signed_signed:
	movq	%rsi, %rcx
	xorl	%eax, %eax
	subq	%rdx, %rcx
	testq	%rdx, %rdx
	jns	.L23
	testq	%rcx, %rsi
	jmp	.L25
.L23:
	orq	%rcx, %rsi
.L25:
	jns	.L21
	movl	$1, %eax
.L21:
	movq	%rcx, (%rdi)
	ret

ckd_mul_unsigned_unsigned_unsigned:
	movq	%rsi, %rax
	mulq	%rdx
	movq	%rax, (%rdi)
	seto	%al
	ret

ckd_mul_signed_signed_signed:
	imulq	%rdx, %rsi
	movq	%rsi, (%rdi)
	seto	%al
	ret

ckd_mul_unsigned_signed_signed:
	xorl	%ecx, %ecx
	movq	%rsi, %rax
	testq	%rdx, %rsi
	jns	.L36
	negq	%rax
	negq	%rdx
	jmp	.L35
.L36:
	xorq	%rdx, %rsi
	jns	.L35
	testq	%rax, %rax
	je	.L35
	xorl	%ecx, %ecx
	testq	%rdx, %rdx
	setne	%cl
.L35:
	mulq	%rdx
	jno	.L33
	movl	$1, %ecx
.L33:
	movq	%rax, (%rdi)
	movl	%ecx, %eax
	ret
	.ident	"GCC: (Alpine 11.2.1_git20220219) 11.2.1 20220219"

========================================================================
jtckdint.h using c11 polyfill
cc -S -Os -std=c11 -o - demo.c
========================================================================


ckd_add_unsigned_unsigned_unsigned:
	addq	%rdx, %rsi
	movq	%rsi, (%rdi)
	setc	%al
	ret

ckd_add_signed_signed_signed:
	leaq	(%rdx,%rsi), %rcx
	movq	%rsi, %rax
	xorq	%rcx, %rax
	xorq	%rcx, %rdx
	movq	%rcx, (%rdi)
	andq	%rdx, %rax
	shrq	$63, %rax
	ret

ckd_add_unsigned_signed_signed:
	leaq	(%rdx,%rsi), %rcx
	movq	%rsi, %rax
	xorq	%rcx, %rax
	andq	%rcx, %rsi
	movq	%rcx, (%rdi)
	andq	%rdx, %rax
	xorq	%rsi, %rax
	shrq	$63, %rax
	ret

ckd_sub_unsigned_unsigned_unsigned:
	movq	%rsi, %rax
	subq	%rdx, %rax
	cmpq	%rdx, %rsi
	movq	%rax, (%rdi)
	setb	%al
	ret

ckd_sub_signed_signed_signed:
	movq	%rsi, %rcx
	movq	%rdx, %rax
	subq	%rdx, %rcx
	xorq	%rsi, %rax
	xorq	%rcx, %rsi
	movq	%rcx, (%rdi)
	andq	%rsi, %rax
	shrq	$63, %rax
	ret

ckd_sub_unsigned_signed_signed:
	movq	%rsi, %rcx
	movq	%rsi, %rax
	subq	%rdx, %rcx
	xorq	%rcx, %rax
	orq	%rcx, %rsi
	movq	%rcx, (%rdi)
	andq	%rdx, %rax
	xorq	%rsi, %rax
	shrq	$63, %rax
	ret

ckd_mul_unsigned_unsigned_unsigned:
	movq	%rsi, %rax
	mulq	%rdx
	movq	%rax, (%rdi)
	seto	%al
	ret

ckd_mul_signed_signed_signed:
	movq	%rdx, %rax
	xorl	%ecx, %ecx
	imulq	%rsi, %rax
	movq	%rax, (%rdi)
	seto	%cl
	testq	%rdx, %rdx
	jns	.L19
	movl	$1, %edi
	movl	$1, %eax
	salq	$63, %rdi
	cmpq	%rdi, %rsi
	je	.L15
.L19:
	xorl	%eax, %eax
	testq	%rdx, %rdx
	cmovne	%ecx, %eax
.L15:
	ret

ckd_mul_unsigned_signed_signed:
	movq	%rsi, %rax
	testq	%rsi, %rdx
	jns	.L23
	negq	%rax
	negq	%rdx
	xorl	%ecx, %ecx
	jmp	.L24
.L23:
	movq	%rdx, %rsi
	xorl	%ecx, %ecx
	xorq	%rax, %rsi
	jns	.L24
	testq	%rax, %rax
	setne	%sil
	xorl	%ecx, %ecx
	testq	%rdx, %rdx
	setne	%cl
	andl	%esi, %ecx
.L24:
	mulq	%rdx
	movq	%rax, (%rdi)
	seto	%al
	movzbl	%al, %eax
	orl	%ecx, %eax
	ret
	.ident	"GCC: (Alpine 11.2.1_git20220219) 11.2.1 20220219"

========================================================================
jtckdint.h using c++11 polyfill
c++ -S -Os -std=c++11 -xc++ -o - demo.c
========================================================================


ckd_add_unsigned_unsigned_unsigned:
	addq	%rdx, %rsi
	movq	%rsi, (%rdi)
	setc	%al
	ret

ckd_add_signed_signed_signed:
	leaq	(%rsi,%rdx), %rcx
	movq	%rsi, %rax
	xorq	%rcx, %rax
	xorq	%rcx, %rdx
	movq	%rcx, (%rdi)
	andq	%rdx, %rax
	shrq	$63, %rax
	ret

ckd_add_unsigned_signed_signed:
	leaq	(%rsi,%rdx), %rcx
	movq	%rsi, %rax
	xorq	%rcx, %rax
	andq	%rcx, %rsi
	movq	%rcx, (%rdi)
	andq	%rdx, %rax
	xorq	%rsi, %rax
	shrq	$63, %rax
	ret

ckd_sub_unsigned_unsigned_unsigned:
	movq	%rsi, %rax
	subq	%rdx, %rax
	cmpq	%rdx, %rsi
	movq	%rax, (%rdi)
	setb	%al
	ret

ckd_sub_signed_signed_signed:
	movq	%rsi, %rcx
	movq	%rdx, %rax
	subq	%rdx, %rcx
	xorq	%rsi, %rax
	xorq	%rcx, %rsi
	movq	%rcx, (%rdi)
	andq	%rsi, %rax
	shrq	$63, %rax
	ret

ckd_sub_unsigned_signed_signed:
	movq	%rsi, %rcx
	movq	%rsi, %rax
	subq	%rdx, %rcx
	xorq	%rcx, %rax
	orq	%rcx, %rsi
	movq	%rcx, (%rdi)
	andq	%rdx, %rax
	xorq	%rsi, %rax
	shrq	$63, %rax
	ret

ckd_mul_unsigned_unsigned_unsigned:
	movq	%rsi, %rax
	mulq	%rdx
	movq	%rax, (%rdi)
	seto	%al
	ret

ckd_mul_signed_signed_signed:
	movq	%rsi, %rax
	movq	%rdi, %r8
	xorl	%edi, %edi
	movq	%rdx, %rcx
	imulq	%rdx, %rax
	movq	%rax, (%r8)
	movl	$1, %eax
	seto	%dil
	shrq	$63, %rcx
	salq	$63, %rax
	cmpq	%rax, %rsi
	sete	%al
	andb	%cl, %al
	jne	.L15
	testq	%rdx, %rdx
	cmovne	%edi, %eax
.L15:
	ret

ckd_mul_unsigned_signed_signed:
	movq	%rdx, %rax
	testq	%rdx, %rsi
	jns	.L21
	negq	%rsi
	negq	%rax
	xorl	%ecx, %ecx
	jmp	.L22
.L21:
	movq	%rsi, %rdx
	xorl	%ecx, %ecx
	xorq	%rax, %rdx
	jns	.L22
	testq	%rsi, %rsi
	setne	%dl
	xorl	%ecx, %ecx
	testq	%rax, %rax
	setne	%cl
	andl	%edx, %ecx
.L22:
	mulq	%rsi
	movq	%rax, (%rdi)
	seto	%al
	movzbl	%al, %eax
	orl	%ecx, %eax
	ret
	.ident	"GCC: (Alpine 11.2.1_git20220219) 11.2.1 20220219"

```

## Alternatives

Consider checking out Kamilcuk's [ckd](https://gitlab.com/Kamcuk/ckd)
library which uses Jinja2 templates to generate an implementation for
each type combination.

## See Also

For further details on checked arithmetic, see the C23 standard:
<https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3096.pdf>

## Funding

Funding for our faster, safer integer arithmetic came from:

- [Mozilla's MIECO program](https://future.mozilla.org/mieco/)
- [jart's GitHub sponsors](https://github.com/sponsors/jart/)
- [jart's Patreon subscribers](https://www.patreon.com/jart)

Your support is what makes this work possible. Thank you!
