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
#include <sstream>

// Token “kind” values:
char const number = '8';    // a floating-point number
char const quit = 'q';      // an exit command
char const print = ';';     // a print command
char const name = 'a';      // name token
char const let = 'L';       // declaration token

std::map<std::string, std::string> expressions; // To store expressions

class token
{
    char kind_;       // what kind of token
    double value_;    // for numbers: a value
    std::string name_; // for variable names

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

std::string const prompt = "> ";
std::string const result = "= "; // indicate that a result follows

class token_stream
{
    std::istream& is;
    token buffer;
    bool full;

public:
    token_stream(std::istream& is)
      : is(is), buffer('\0'), full(false)
    {
    }

    token get();
    void putback(token);
    void ignore(char c);
    std::string skip_to(char c);
};

template<typename T, typename U>
T narrow_cast(U&& u) {
    return static_cast<T>(std::forward<U>(u));
}

void token_stream::putback(token t)
{
    if (full)
        throw std::runtime_error("putback() into a full buffer");
    buffer = t;
    full = true;
}

token token_stream::get()
{
    // check if we already have a Token ready
    if (full)
    {
        full = false;
        return buffer;
    }

    char ch;
    is >> ch;

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
    case '%':       // Added support for modulo operator
    case '=':       // Added support for variable assignment
        return token(ch);       // let each character represent itself
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
        is.putback(ch);     // put digit back into the input stream
        double val;
        is >> val;          // read a floating-point number
        return token(val);
    }
    default:
        if (isalpha(ch)) {      // If it's a letter
            std::string s;
            s += ch;
            while (is.get(ch) && (isalpha(ch) || isdigit(ch))) {
                s += ch;
            }
            is.putback(ch);
            return token(name, s);
        }
        throw std::runtime_error("Bad token");
    }
}

void token_stream::ignore(char c)
{
    char ch = 0;
    while (is >> ch)
    {
        if (ch == c)
            break;
    }
}

std::string token_stream::skip_to(char c)
{
    std::string result;
    char ch = 0;
    while (is >> ch)
    {
        result += ch;
        if (ch == c)
            break;
    }
    return result;
}

double expression(token_stream& ts);

double primary(token_stream& ts)    // Number or ‘(‘ Expression ‘)’
{
    token t = ts.get();
    switch (t.kind())
    {
    case '(':           // handle ‘(‘ Expression ‘)’
    {
        double d = expression(ts);
        t = ts.get();
        if (t.kind() != ')')
            throw std::runtime_error("')' expected");
        return d;
    }
    case number:            // we use ‘8’ to represent the “kind” of a number
        return t.value();   // return the number’s value
    case name:
    {
        std::string expr;
        token next_token = ts.get();
        if (next_token.kind() == '=')
        {
            expr = ts.skip_to(print);
            expressions[t.name()] = expr;
        }
        else
        {
            ts.putback(next_token);
            auto it = expressions.find(t.name());
            if (it == expressions.end())
            {
                throw std::runtime_error("Undefined variable");
            }
            expr = it->second;
        }

        std::istringstream iss(expr);
        token_stream sts(iss);
        return expression(sts);
    }
    case '-':
        return -primary(ts);    // handles negative number
    default:
        throw std::runtime_error("primary expected");
    }
}

double term(token_stream& ts)
{
    double left = primary(ts);    // get the Primary
    while (true)
    {
        token t = ts.get();         // get the next Token ...
        switch (t.kind())
        {
        case '*':
            left *= primary(ts);
            break;
        case '/':
        {
            double d = primary(ts);
            if (d == 0)
                throw std::runtime_error("divide by zero");
            left /= d;
            break;
        }
        case '%':
        {
            int i1 = narrow_cast<int>(left);
            int i2 = narrow_cast<int>(term());
            if (i2 == 0) throw std::runtime_error("divide by zero");
            left = i1 % i2;
            break;
        }
        default:
            ts.putback(t);       // <<< put the unused token back
            return left;        // return the value
        }
    }
}

double expression(token_stream& ts)
{
    double left = term(ts);    // get the term
    while (true)
    {
        token t = ts.get();         // get the next Token ...
        switch (t.kind())           // ... and do the right thing with it
        {
        case '+':
            left += term(ts);
            break;
        case '-':
            left -= term(ts);
            break;
        default:
            ts.putback(t);       // <<< put the unused token back
            return left;        // return the value of the expression
        }
    }
}

void clean_up_mess(token_stream& ts)
{
    ts.ignore(print);
}

void calculate(token_stream& ts)
{
    while (true)
    {
        try
        {
            std::cout << prompt;         // print prompt
            token t = ts.get();

            while (t.kind() == print)
                t = ts.get();

            if (t.kind() == quit)       
                return;                 // 'q' to quit

            ts.putback(t);

            std::cout << result << expression(ts) << std::endl;
        }
        catch (std::runtime_error const& e)
        {
            std::cerr << e.what() << std::endl; // write error message
            clean_up_mess();                    // <<< The tricky part!
        }
    }
}

int main()
{
    try
    {
        token_stream sts(std::cin);
        calculate(sts);
        return 0;
    }
    catch (...)
    {
         // other errors (don't try to recover)
        std::cerr << "exception\n";
        return 2;
    }
}