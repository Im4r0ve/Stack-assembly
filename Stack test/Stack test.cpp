
#include <iostream>
#include <string>
#include <vector>
using namespace std;

enum class token_type
{
	integer,
	plus,
	krat,
	zavorkaO,
	zavorkaZ
};

struct Token
{
	token_type type;
	int value;
	Token(token_type type, int value = 0)
		: type(type)
		, value(value)
	{}
	string format()
	{
		switch (type) {
		case token_type::integer:
			return "integer " + to_string(value);
		case token_type::plus:
			return "+";
		case token_type::krat:
			return "*";
		case token_type::zavorkaO:
			return "(";
		case token_type::zavorkaZ:
			return ")";
		default:
			return "?";
		}
	}
};

vector<Token>
tokenize(istream& in)
{
	char c;
	vector<Token> res;
	while (in) {
		c = in.peek();
		if (c < 0) {
			continue;
		}
		if (isspace(c)) {
			in.get();
			continue;
		}
		if (c >= '0' && c <= '9') {
			int n;
			in >> n;
			res.emplace_back(token_type::integer, n);
			continue;
		}
		switch (c) {
		case '+':
			res.emplace_back(token_type::plus);
			break;
		case '*':
			res.emplace_back(token_type::krat);
			break;
		case '(':
			res.emplace_back(token_type::zavorkaO);
			break;
		case ')':
			res.emplace_back(token_type::zavorkaZ);
			break;
		default:
			cerr << "Unexpected character!" << c << " "
				<< int(c) << endl;
		}
		in.get();
	}
	return res;
}

class Expr
{
public:
	virtual string format() const = 0;
	virtual int eval() const = 0;
};

class Num : public Expr
{
	int val;

public:
	Num(int v)
		: val(v)
	{}

	string format() const { return to_string(val); }

	int eval() const { return val; }
};

class Add : public Expr
{
	unique_ptr<Expr> left, right;

public:
	Add(unique_ptr<Expr>&& left, unique_ptr<Expr>&& right)
		: left(move(left))
		, right(move(right))
	{}
	int eval() const { return left->eval() + right->eval(); }
	string format() const
	{
		return "(" + left->format() + " + " + right->format() + ")";
	}
};

class Mult : public Expr
{
	unique_ptr<Expr> left, right;

public:
	Mult(unique_ptr<Expr>&& left, unique_ptr<Expr>&& right)
		: left(move(left))
		, right(move(right))
	{}
	int eval() const { return left->eval() * right->eval(); }
	string format() const
	{
		return left->format() + " * " + right->format();
	}
};

using Pos = vector<Token>::iterator;

unique_ptr<Expr>
ParseSimpleExpr(Pos& begin, Pos end)
{
	if (begin == end)
		return nullptr;
	if (begin->type == token_type::integer) {
		unique_ptr<Expr> val = make_unique<Num>(begin->value);
		begin++;
		return val;
	}
	return nullptr;
}

unique_ptr<Expr>
ParseMulExpr(Pos& begin, Pos end)
{
	unique_ptr<Expr> l = ParseSimpleExpr(begin, end);
	if (!l)
		return l;
	if (begin == end)
		return l;
	if (begin->type != token_type::krat)
		return l;
	begin++;
	unique_ptr<Expr> r = ParseMulExpr(begin, end);
	if (!r) {
		begin--;
		return l;
	}
	return make_unique<Mult>(move(l), move(r));
}

unique_ptr<Expr>
ParseAddExpr(Pos& begin, Pos end)
{
	unique_ptr<Expr> l = ParseMulExpr(begin, end);
	if (!l)
		return l;
	if (begin == end)
		return l;
	if (begin->type != token_type::plus)
		return l;
	begin++;
	unique_ptr<Expr> r = ParseAddExpr(begin, end);
	if (!r) {
		begin--;
		return l;
	}
	return make_unique<Add>(move(l), move(r));
}

unique_ptr<Expr>
ParseExpr(Pos begin, Pos end)
{
	return ParseAddExpr(begin, end);
}

int
main()
{
	auto vec = tokenize(cin);
	for (auto& i : vec) {
		cout << i.format() << endl;
	}
	auto e = ParseExpr(vec.begin(), vec.end());
	cout << e->format() << endl;
}
