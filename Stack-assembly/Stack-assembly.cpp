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
	identifier,
	integer,	
	plus,
	minus,
	times,
	paren_open,
	paren_close,
	curly_open,
	curly_close,
	input,
	output,
	assign,
	T_condition,
	F_condition,
	cycle,
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
		case '{':
			res.emplace_back(token_type::curly_open);
			break;
		case '}':
			res.emplace_back(token_type::curly_close);
			break;
		case '>':
			res.emplace_back(token_type::input);
			break;
		case '<':
			res.emplace_back(token_type::output);
			break;
		case '=':
			res.emplace_back(token_type::assign);
			break;
		case '?':
			res.emplace_back(token_type::T_condition);
			break;
		case '!':
			res.emplace_back(token_type::F_condition);
			break;
		case '@':
			res.emplace_back(token_type::cycle);
			break; 
		case ';':
			res.emplace_back(token_type::semicolon);
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
	virtual string toAssembly(size_t& counter) const = 0;
};

class Prog
{ 
public: 
	virtual string toAssembly(size_t& counter) const = 0;
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
	string toAssembly(size_t& counter) const
	{	
		return left->toAssembly(counter) + right->toAssembly(counter);
	}
};

class In : public Prog
{
	string name;

public:
	In(const string& name)
		: name(name)
	{}
	string toAssembly(size_t& counter) const
	{
		counter += 2;
		return	"READ\n"
			"STOREVAR " + name + "\n";
	}
};

class Out : public Prog
{
	string name;

public:
	Out(const string& name)
		: name(name)
	{}
	string toAssembly(size_t& counter) const
	{
		counter += 2;
		return	"LOADVAR " + name +
				"\nWRITE\n";
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
	string toAssembly(size_t& counter) const
	{
		counter++;
		return	right->toAssembly(counter) + 
				"STOREVAR " + name + "\n";
	}
};

class T_condition : public Prog
{
	string name;
	unique_ptr<Prog> right;

public:
	T_condition(const string& name, unique_ptr<Prog>&& right)
		: name(name)
		, right(move(right))
	{}
	string toAssembly(size_t& counter) const
	{
		size_t orig_counter = counter;
		counter += 2;
		return	"LOADVAR " + name + "\n" + 
				"JMPF " + to_string(counter - orig_counter-1) + "\n" +
				right->toAssembly(counter);
	}
};

class F_condition : public Prog
{
	string name;
	unique_ptr<Prog> right;

public:
	F_condition(const string& name, unique_ptr<Prog>&& right)
		: name(name)
		, right(move(right))
	{}
	string toAssembly(size_t& counter) const
	{
		size_t orig_counter = counter;
		counter += 2;
		return	"LOADVAR " + name + "\n" + 
				"JMPT " + to_string(counter - orig_counter-1) + "\n" +
				right->toAssembly(counter);
	}
};

class Cycle : public Prog
{
	string name;
	unique_ptr<Prog> right;

public:
	Cycle(const string& name, unique_ptr<Prog>&& right)
		: name(name)
		, right(move(right))
	{}
	string toAssembly(size_t& counter) const
	{
		size_t orig_counter = counter;
		counter += 2;
		string recurse = right->toAssembly(counter);
		counter++;
		return	"LOADVAR " + name + "\n"+ 
				"JMPF " + to_string(counter - orig_counter - 1) + "\n" +
				recurse + 
				"JMP -" + to_string(counter - orig_counter - 1) + "\n";
	}
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

	string toAssembly(size_t& counter) const
	{
		counter++;
		return "INT " + to_string(val) + "\n";
	}
};

class Var : public Expr
{
	string name;
public:
	Var(const string& v)
		: name(v) {}

	string toAssembly(size_t& counter) const
	{
		counter++;
		return "LOADVAR " + name + "\n";
	}
};

//class for operations: add, subtract and multiply
class BinOp : public Expr
{
	token_type op;
	unique_ptr<Expr> left, right;

public:
	BinOp(unique_ptr<Expr>&& left, unique_ptr<Expr>&& right, token_type op)
		: left(move(left))
		, right(move(right))
		, op(op)
	{}

	string toAssembly(size_t& counter) const
	{
		counter++;
		string operation = (op == token_type::plus) ? "ADD" : ((op == token_type::minus) ? "SUB" : "MULT");
		return left->toAssembly(counter) + right->toAssembly(counter) + operation + "\n";
	}
};

/*
 * Parsing
 */

using Pos = vector<Token>::iterator;

unique_ptr<Expr>
ParseExpr(Pos& begin, Pos end);
unique_ptr<Prog>
ParseProg(Pos& begin, Pos end);

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
		//if you need to change between + and -
		if (operation != token_type::times && 
			begin->type != token_type::times)
		{
			operation = (operation == token_type::plus) ? token_type::minus : token_type::plus;
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

	return make_unique<BinOp>(move(l), move(r), operation);	
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

	switch (begin->type)
	{
	case token_type::input:
	{
		begin++;
		if (begin == end || begin->type != token_type::identifier) {
			begin--;
			return nullptr;
		}

		return make_unique<In>((begin++)->name);
	}
	case  token_type::output:
	{
		begin++;
		if (begin == end || begin->type != token_type::identifier) {
			begin--;
			return nullptr;
		}

		return make_unique<Out>((begin++)->name);
	}
	case  token_type::assign:
	{
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
	case  token_type::cycle:
	{
		begin++;
		if (begin == end || begin->type != token_type::identifier) {
			begin--;
			return nullptr;
		}
		string n = (begin++)->name;
		unique_ptr<Prog> e = ParseSimpleProg(begin, end);

		if (!e) {
			begin -= 2;
			return nullptr;
		}

		return make_unique<Cycle>(n, move(e));
	}
	case  token_type::T_condition:
	{
		begin++;
		if (begin == end || begin->type != token_type::identifier) {
			begin--;
			return nullptr;
		}

		string n = (begin++)->name;
		unique_ptr<Prog> e = ParseSimpleProg(begin, end);

		if (!e) {
			begin -= 2;
			return nullptr;
		}

		return make_unique<T_condition>(n, move(e));
	}
	case  token_type::F_condition:
	{
		begin++;
		if (begin == end || begin->type != token_type::identifier) {
			begin--;
			return nullptr;
		}
		string n = (begin++)->name;
		unique_ptr<Prog> e = ParseSimpleProg(begin, end);
		if (!e) {
			begin -= 2;
			return nullptr;
		}

		return make_unique<F_condition>(n, move(e));
	}
	case  token_type::curly_open:
	{
		Pos original_begin = begin;
		begin++;

		unique_ptr<Prog> e = ParseProg(begin, end);
		if (!e || begin == end ||
			begin->type != token_type::curly_close) {
			begin = original_begin;
			return nullptr;
		}

		begin++;
		return e;
	}
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

int main()
{
	auto tokens = tokenize(cin);
	auto b = tokens.begin();
	auto e = ParseProg(b, tokens.end());

	//if there is mistake in code only write FAIL
	if (!e || b != tokens.end())
	{
		cout << "FAIL";
		return 0;
	}

	//print assembly
	size_t counter = 0;
	cout << e->toAssembly(counter);
	cout << "QUIT" << endl;

	return 0;
}