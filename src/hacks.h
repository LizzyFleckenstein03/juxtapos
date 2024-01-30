#define NONE(...)
#define ID(X, ...) X
#define CALL(F, ...) F(__VA_ARGS__)
#define EXPAND(...) __VA_ARGS__
#define BRACE(...) (__VA_ARGS__)

#define IF_0(t, f) f
#define IF_1(t, f) t
#define IF_(cond) IF_##cond
#define IF(cond, t, f) IF_(cond)(t, f)

// add more iterations as needed
#define FOLD_(F, init, x0, x1, x2, x3, x4, x5, x6, x7, ...) \
	F(F(F(F(F(F(F(F(init, x0), x1), x2), x3), x4), x5), x6), x7)
#define FOLD(NIL,...) FOLD_(__VA_ARGS__,NIL,NIL,NIL,NIL,NIL,NIL,NIL,NIL,NIL,NIL)

#define GET1(x, ...) x
#define GET2(x, ...) GET1(__VA_ARGS__)
#define GET3(x, ...) GET2(__VA_ARGS__)

// add more EQUALS_X_X as needed
#define EQUALS_0_0 ~, 1
#define EQUALS_1_1 ~, 1
#define EQUALS_2_2 ~, 1
#define EQUALS(X, Y) ID(GET2 BRACE(EQUALS_##X##_##Y, 0))

/*
#define COMMA(...) ,
#define HAS_COMMA(...) HAS_COMMA_1(__VA_ARGS__, COMMA(), ~)
#define HAS_COMMA_1(A, B, ...) HAS_COMMA_2(B, 0, 1, ~)
#define HAS_COMMA_2(A, B, C, ...) C

#define TRUE(...) 1
#define FALSE(...) 0
#define NOT(X) IF(X, 0, 1)
#define OR(A, B) IF(A, 1, B)
#define AND(A, B) IF(A, B, 0)

#define IS_CALL(X, ...) HAS_COMMA(ID(COMMA X))

#define AND3(A, B, C) AND(A, AND(B, C))

#define IS_EMPTY(...) AND3( \
	NOT(HAS_COMMA(__VA_ARGS__)), \
	NOT(IS_CALL(__VA_ARGS__)), \
	HAS_COMMA(COMMA __VA_ARGS__ ()))
*/

#define T_BYTE (1, GLbyte, GL_BYTE)
#define T_UBYTE (1, GLubyte, GL_UNSIGNED_BYTE)
#define T_SHORT (1, GLshort, GL_SHORT)
#define T_USHORT (1, GLushort, GL_UNSIGNED_SHORT)
#define T_INT (1, GLint, GL_INT)
#define T_UINT (1, GLuint, GL_UNSIGNED_INT)
#define T_FIXED (1, GLfixed, GL_FIXED)
#define T_HALF (1, GLhalf, GL_HALF_FLOAT)
#define T_FLOAT (1, GLfloat, GL_FLOAT)
#define T_DOUBLE (1, GLdouble, GL_DOUBLE)

#define T_PRESENT(x) ID(GET1 x)
#define T_TYPE(x) ID(GET2 x)
#define T_CONST(x) ID(GET3 x)

#define VERTEX_DEF_FOLD(struct_name, buf_struct, buf_func, index, type, name, count, ...) ( \
	struct_name, \
	buf_struct T_TYPE(type) name IF(EQUALS(count, 1),,[count]);, \
	buf_func \
		glVertexAttribPointer(index, count, T_CONST(type), \
			GL_FALSE, sizeof(struct struct_name), (GLvoid *) offsetof(struct struct_name, name)); \
		glEnableVertexAttribArray(index);, \
	(1 + index))

#define VERTEX_DEF_FOLD_WRAP(buf, attr) \
	IF(T_PRESENT(GET1 attr), CALL(VERTEX_DEF_FOLD, EXPAND buf, EXPAND attr, 1), buf)

#define VERTEX_DEF_EMIT(name, buf_struct, buf_func, _) \
	struct name { buf_struct }; \
	void name ## _configure_vao() { buf_func };

#define VERTEX_DEF(name, ...) ID(VERTEX_DEF_EMIT FOLD(((0, ~, ~), ~, 0), VERTEX_DEF_FOLD_WRAP, \
	 (name,,, 0), __VA_ARGS__))
