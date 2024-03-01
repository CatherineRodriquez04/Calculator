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

public:
    token_stream(std::istream& is)
      : is(is)
    {
    }

    token get();
    void putback(token);
    void ignore(char c);
};

template<typename T, typename U>
T narrow_cast(U&& u) {
    return static_cast<T>(std::forward<U>(u));
}

token_stream ts(std::cin);

void token_stream::putback(token t)
{
    is.putback(t.kind());
}

token token_stream::get()
{
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

double expression();

double primary()        // Number or ‘(‘ Expression ‘)’
{
    token t = ts.get();
    switch (t.kind())
    {
    case '(':           // handle ‘(‘ Expression ‘)’
    {
        double d = expression();
        t = ts.get();
        if (t.kind() != ')')
            throw std::runtime_error("')' expected");
        return d;
    }
    case number:            // we use ‘8’ to represent the “kind” of a number
        return t.value();   // return the number’s value
    case name:
    {
        token next_token = ts.get();
        if (next_token.kind() == '=')
        {
            double value = expression();
            expressions[t.name()] = std::to_string(value);
            return value;
        }
        else
        {
            ts.putback(next_token);
            if (expressions.find(t.name()) != expressions.end())
            {
                std::istringstream iss(expressions[t.name()]);
                double result;
                iss >> result;
                return result;
            }
            else
            {
                throw std::runtime_error("Undefined variable");
            }
        }
    }
    case '-':
        return -primary();      // handles negative number
    default:
        throw std::runtime_error("primary expected");
    }
}

double term()
{
    double left = primary();        // get the Primary
    while (true)
    {
        token t = ts.get();         // get the next Token ...
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

double expression()
{
    double left = term();           // get the term
    while (true)
    {
        token t = ts.get();         // get the next Token ...
        switch (t.kind())           // ... and do the right thing with it
        {
        case '+':
            left += term();
            break;
        case '-':
            left -= term();
            break;
        default:
            ts.putback(t);       // <<< put the unused token back
            return left;        // return the value of the expression
        }
    }
}

void clean_up_mess()
{
    ts.ignore(print);
}

void calculate()
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

            std::cout << result << expression() << std::endl;
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