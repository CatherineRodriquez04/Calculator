/*
Catherine Rodriquez
CSC 3380
Calculator Project:
    - +, -, *. /, (, ), =
    - stores variables
        - updates variable when reference is changed
    - negative numbers
    - % (modulus/remainder)
    - pre-defined symbolic values 
*/


#include <iostream>
#include <stdexcept>
#include <string>
#include <map> //store variables

// Token stuff
// Token “kind” values:
char const number = '8';    // a floating-point number
char const quit = 'q';      // an exit command
char const print = ';';     // a print command
char const name = 'a';      // name token
char const let = 'L';       // delclaration token

std::map<std::string, double> variables; // To store variables

class token
{
    char kind_;       // what kind of token
    double value_;    // for numbers: a value
    std::string name_; //for variable names

public:
    // constructors
    token(char ch)
      : kind_(ch)
      , value_(0)
    {
    }
    token(double val)
      : kind_(number)    // let ‘8’ represent “a number”
      , value_(val)
    {
    }
    token(char ch, const std::string& name)
      : kind_(ch)
      , name_(name)
    {
    }

    char kind() const
    {
        return kind_;
    }
    double value() const
    {
        return value_;
    }
    std::string name() const
    {
        return name_;
    }
};

// User interaction strings:
std::string const prompt = "> ";
std::string const result = "= ";    // indicate that a result follows

class token_stream
{
    // representation: not directly accessible to users:
    bool full;       // is there a token in the buffer?
    token buffer;    // here is where we keep a Token put back using
                     // putback()
public:
    // user interface:
    token get();            // get a token
    void putback(token);    // put a token back into the token_stream
    void ignore(char c);    // discard tokens up to and including a c

    // constructor: make a token_stream, the buffer starts empty
    token_stream()
      : full(false)
      , buffer('\0')
    {
    }
};

template<typename T, typename U>
T narrow_cast(U&& u) {
    return static_cast<T>(std::forward<U>(u));
}

// single global instance of the token_stream
token_stream ts;

void token_stream::putback(token t)
{
    if (full)
        throw std::runtime_error("putback() into a full buffer");
    buffer = t;
    full = true;
}

token token_stream::get()    // read a token from the token_stream
{
    // check if we already have a Token ready
    if (full)
    {
        full = false;
        return buffer;
    }

    // note that >> skips whitespace (space, newline, tab, etc.)
    char ch;
    std::cin >> ch;

    switch (ch)
    {
    case '(':
    case ')':
    case ';':
    case 'q':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':  // Added support for modulo operator
    case '=':  // Added support for variable assignment
        return token(ch);    // let each character represent itself
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
        std::cin.putback(ch);    // put digit back into the input stream
        double val;
        std::cin >> val;    // read a floating-point number
        return token(val);
    }
    default:
        if (isalpha(ch)) { // If it's a letter
            std::string s;
            s += ch;
            while (std::cin.get(ch) && (isalpha(ch) || isdigit(ch))) {
                s += ch;
            }
            std::cin.putback(ch);
            return token(name, s);
        }
        throw std::runtime_error("Bad token");
    }
}

// discard tokens up to and including a c
void token_stream::ignore(char c)
{
    // first look in buffer:
    if (full && c == buffer.kind())    // && means 'and'
    {
        full = false;
        return;
    }
    full = false;    // discard the contents of buffer

    // now search input:
    char ch = 0;
    while (std::cin >> ch)
    {
        if (ch == c)
            break;
    }
}

// declaration so that primary() can call expression()
double expression();

double primary()    // Number or ‘(‘ Expression ‘)’
{
    token t = ts.get();
    switch (t.kind())
    {
    case '(':    // handle ‘(’expression ‘)’
    {
        double d = expression();
        t = ts.get();
        if (t.kind() != ')')
            throw std::runtime_error("')' expected");
        return d;
    }
    case number:    // we use ‘8’ to represent the “kind” of a number
        return t.value();    // return the number’s value
    case name:
        {
            token next_token = ts.get();
            if (next_token.kind() == '=') {
                double value = expression();
                variables[t.name()] = value;
                return value;
            } else {
                ts.putback(next_token);
                return variables[t.name()];
            }
        }
    case '-':
        return -primary(); // Handle negative numbers
    default:
        throw std::runtime_error("primary expected");
    }
}

// exactly like expression(), but for * and /
double term()
{
    double left = primary();    // get the Primary
    while (true)
    {
        token t = ts.get();    // get the next Token ...
        switch (t.kind())
        {
        case '*':
            left *= primary();
            break;
        case '/':
        {
            double d = primary();
            if (d == 0)
                throw std::runtime_error("divide by zero");
            left /= d;
            break;
        }
        case '%': // Added modulo operation
        {
            int i1 = narrow_cast<int>(left);
            int i2 = narrow_cast<int>(term());
            if (i2 == 0) throw std::runtime_error("divide by zero");
            left = i1 % i2;
            break;
        }
        default:
            ts.putback(t);    // <<< put the unused token back
            return left;      // return the value
        }
    }
}

// read and evaluate: 1   1+2.5   1+2+3.14  etc.
// 	 return the sum (or difference)
double expression()
{
    double left = term();    // get the Term
    while (true)
    {
        token t = ts.get();    // get the next token…
        switch (t.kind())      // ... and do the right thing with it
        {
        case '+':
            left += term();
            break;
        case '-':
            left -= term();
            break;
        default:
            ts.putback(t);    // <<< put the unused token back
            return left;      // return the value of the expression
        }
    }
}

void clean_up_mess()
{
    ts.ignore(print);
}

void calculate()
{
    while (std::cin)
    {
        try
        {
            std::cout << prompt;    // print prompt
            token t = ts.get();

            // first discard all “prints”
            while (t.kind() == print)
                t = ts.get();

            if (t.kind() == quit)
                return;    // ‘q’ for “quit”

            ts.putback(t);

            std::cout << result << expression() << std::endl;
        }
        catch (std::runtime_error const& e)
        {
            std::cerr << e.what() << std::endl;    // write error message
            clean_up_mess();                       // <<< The tricky part!
        }
    }
}

int main()
{
    try
    {
        calculate();
        return 0;
    }
    catch (...)
    {
        // other errors (don't try to recover)
        std::cerr << "exception\n";
        return 2;
    }
}