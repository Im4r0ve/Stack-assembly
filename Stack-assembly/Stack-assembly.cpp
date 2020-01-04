#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;

/*
 * tokenizer
 */

enum class token_type
{
	integer,
	identifier,
	plus,
	minus,
	times,
	paren_open,
	paren_close,
	input,
	output,
	assign,
	semicolon
};

struct Token
{
	token_type type;
	int value;
	string name;
	Token(token_type type, int value = 0)
		: type(type)
		, value(value)
	{}
	Token(string&& name)
		: type(token_type::identifier)
		, value(0)
		, name(name)
	{}
	string format()
	{
		switch (type) {
		case token_type::integer:
			return to_string(value);
		case token_type::plus:
			return "+";
		case token_type::minus:
			return "-";
		case token_type::times:
			return "*";
		case token_type::paren_open:
			return "(";
		case token_type::paren_close:
			return ")";
		case token_type::identifier:
			return name;
		case token_type::input:
			return ">";
		case token_type::output:
			return "<";
		case token_type::semicolon:
			return ";";
		case token_type::assign:
			return "=";
		default:
			return "Fail";
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
		if (c == '.') {
			//use instead of EOF
			in.get();
			break;
		}
		if (isspace(c)) {
			in.get();
			continue;
		}
		if (isdigit(c)) {
			int n;
			in >> n;
			res.emplace_back(token_type::integer, n);
			continue;
		}
		if (islower(c)) {
			string s;
			while (islower(in.peek())) {
				s += in.get();
			}
			res.emplace_back(move(s));
			continue;
		}

		switch (c) {
		case '+':
			res.emplace_back(token_type::plus);
			break;
		case '-':
			res.emplace_back(token_type::minus);
			break;
		case '*':
			res.emplace_back(token_type::times);
			break;
		case '(':
			res.emplace_back(token_type::paren_open);
			break;
		case ')':
			res.emplace_back(token_type::paren_close);
			break;
		case '<':
			res.emplace_back(token_type::output);
			break;
		case '>':
			res.emplace_back(token_type::input);
			break;
		case ';':
			res.emplace_back(token_type::semicolon);
			break;
		case '=':
			res.emplace_back(token_type::assign);
			break;
		default:
			cerr << "Unexpected character: '" << c << "' " << int(c)
				<< endl;
		}
		in.get();
	}
	return res;
}

/*
 * interfaces for AST
 * (very abstract now)
 */

class Expr
{ 
public:
	virtual string format() const = 0;
	virtual int eval() const = 0;
};

class Prog
{ 
	virtual void compute() const = 0;

};

/*
 * program implementations
 */
class Seq : public Prog
{
	unique_ptr<Prog> left, right;

public:
	Seq(unique_ptr<Prog>&& left, unique_ptr<Prog>&& right)
		: left(move(left))
		, right(move(right))
	{}
	void compute()
	{	
	}
};

class Out : public Prog
{
	string name;

public:
	Out(const string& name)
		: name(name)
	{}
	void compute()
	{
		cout << "WRITE" << endl;
	}
};

class In : public Prog
{
	string name;

public:
	In(const string& name)
		: name(name)
	{}
	void compute()
	{
		cout << "READ" << endl;
	}
};

class Assign : public Prog
{

	string name;
	unique_ptr<Expr> right;

public:
	Assign(const string& name, unique_ptr<Expr>&& right)
		: name(name)
		, right(move(right))
	{}
};

/*
 * expressions
 */

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

class Var : public Expr
{
	string name;
public:
	Var(const string& v)
		: name(v) {}

	string format() const { return name; }

	int eval() const { return 0; }
};

class Add : public Expr
{
	unique_ptr<Expr> left, right;

public:
	Add(unique_ptr<Expr>&& left, unique_ptr<Expr>&& right)
		: left(move(left))
		, right(move(right))
	{}

	int eval() const
	{ 
		return left->eval() + right->eval(); 
	}

	string format() const
	{
		return "(" + left->format() + " + " + right->format() + ")";
	}
};

class Sub : public Expr
{
	unique_ptr<Expr> left, right;

public:
	Sub(unique_ptr<Expr>&& left, unique_ptr<Expr>&& right)
		: left(move(left))
		, right(move(right))
	{}

	int eval() const
	{
		return left->eval() + right->eval();
	}

	string format() const
	{
		return "(" + left->format() + " - " + right->format() + ")";
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

	int eval() const 
	{
		return left->eval() * right->eval(); 
	}

	string format() const
	{
		return "(" + left->format() + " * " + right->format() + ")";
	}
};

/*
 * Parsing
 */

using Pos = vector<Token>::iterator;

unique_ptr<Expr>
ParseExpr(Pos& begin, Pos end);

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

	if (begin->type == token_type::identifier) {
		unique_ptr<Expr> val = make_unique<Var>(begin->name);
		begin++;
		return val;
	}

	if (begin->type == token_type::paren_open) {
		Pos original_begin = begin;
		begin++;

		unique_ptr<Expr> e = ParseExpr(begin, end);
		if (!e || begin == end ||
			begin->type != token_type::paren_close) {
			begin = original_begin;
			return nullptr;
		}

		begin++;
		return e;
	}

	return nullptr;
}

unique_ptr<Expr>
ParseBinOp(Pos& begin, Pos end, token_type operation)
{
	unique_ptr<Expr> l;
	switch (operation)
	{
	case token_type::plus:
		l = ParseBinOp(begin, end, token_type::minus);
		break;
	case token_type::minus:
		l = ParseBinOp(begin, end, token_type::times);
		break;
	case token_type::times:
		l = ParseSimpleExpr(begin, end);
		break;
	}

	if (!l)
		return l;
	if (begin == end)
		return l;
	
	if (begin->type != operation )
	{
		if (operation != token_type::times && begin->type != token_type::times)
		{
			(operation == token_type::plus) ? operation = token_type::minus : operation = token_type::plus;
		}
		else
			return l;
	}
	begin++;
	unique_ptr<Expr> r = ParseBinOp(begin, end, operation);
	if (!r) {
		begin--;
		return l;
	}

	switch (operation)
	{
	case token_type::plus:
		return make_unique<Add>(move(l), move(r));
		break;
	case token_type::minus:
		return make_unique<Sub>(move(l), move(r));
		break;
	case token_type::times:
		return make_unique<Mult>(move(l), move(r));
		break;
	}
}

unique_ptr<Expr>
ParseExpr(Pos& begin, Pos end)
{
	return ParseBinOp(begin, end, token_type::plus);
}

unique_ptr<Prog>
ParseSimpleProg(Pos& begin, Pos end)
{
	if (begin == end)
		return nullptr;
	if (begin->type == token_type::output) {
		begin++;
		if (begin == end || begin->type != token_type::identifier) {
			begin--;
			return nullptr;
		}

		return make_unique<Out>((begin++)->name);
	}
	if (begin->type == token_type::input) {
		begin++;
		if (begin == end || begin->type != token_type::identifier) {
			begin--;
			return nullptr;
		}

		return make_unique<In>((begin++)->name);
	}
	if (begin->type == token_type::assign) {
		begin++;
		if (begin == end || begin->type != token_type::identifier) {
			begin--;
			return nullptr;
		}
		string n = (begin++)->name;
		unique_ptr<Expr> e = ParseExpr(begin, end);
		if (!e) {
			begin -= 2;
			return nullptr;
		}

		return make_unique<Assign>(n, move(e));
	}

	return nullptr;
}

unique_ptr<Prog>
ParseProg(Pos& begin, Pos end)
{
	unique_ptr<Prog> l = ParseSimpleProg(begin, end);
	if (!l)
		return l;
	if (begin == end)
		return l;
	if (begin->type != token_type::semicolon)
		return l;
	begin++;
	unique_ptr<Prog> r = ParseProg(begin, end);
	if (!r) {
		begin--;
		return l;
	}
	return make_unique<Seq>(move(l), move(r));
}

/*
 * the main program
 */

int
main()
{
	auto tokens = tokenize(cin);
	auto b = tokens.begin();
	//auto e = ParseProg(b, tokens.end());
	
		auto e = ParseExpr(b, tokens.end());
		cout << e->format() << endl;
	
	//TODO
	return 0;
}