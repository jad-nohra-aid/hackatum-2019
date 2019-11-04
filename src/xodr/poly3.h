#pragma once

namespace aid { namespace xodr {

/**
 * @brief A cubic polynomial.
 */
class Poly3
{
  public:
    /**
     * @brief Creates an uninitialized polynomial.
     */
    Poly3() = default;

    /**
     * @brief Creates a copy of the given source polynomial.
     */
    Poly3(const Poly3& src) = default;

    /**
     * @brief Moves the given source polynomial.
     */
    Poly3(Poly3&& src) = default;

    /**
     * @brief Creates a polynomial with the given coefficients.
     *
     * The polynomial takes the following form: f(t) = a + bt + ct^2 + dt^3.
     *
     * @param a             The coefficient of the constant term.
     * @param b             The coefficient of the t term.
     * @param c             The coefficient of the t^2 term.
     * @param d             The coefficient of the t^3 term.
     */
    Poly3(double a, double b, double c, double d)
    {
        a_ = a;
        b_ = b;
        c_ = c;
        d_ = d;
    }

    Poly3& operator=(const Poly3& src) = default;

    Poly3& operator=(Poly3&& src) = default;

    /**
     * @brief Compares two polynomials for equality.
     *
     * @param rhs           The right hand side of the comparison.
     * @returns             True if *this and 'rhs' are equal, false otherwise.
     */
    bool operator==(const Poly3& rhs) const { return a_ == rhs.a_ && b_ == rhs.b_ && c_ == rhs.c_ && d_ == rhs.d_; }

    /**
     * @brief Compares two polynomials for inequality.
     *
     * @param rhs           The right hand side of the comparison.
     * @returns             True if *this and 'rhs' are distinct, false otherwise.
     */
    bool operator!=(const Poly3& rhs) const { return !(*this == rhs); }

    /**
     * Adds the given polynomial to this polynomial.
     *
     * @brief b             The polynomial to add.
     * @returns             A reference to this polynomial.
     */
    Poly3 operator+=(const Poly3& b)
    {
        a_ += b.a_;
        b_ += b.b_;
        c_ += b.c_;
        d_ += b.d_;
        return *this;
    }

    /**
     * Adds 2 polynomials.
     *
     * @params b            The second operand.
     * @returns             The sum of the polynomials *this and b.
     */
    Poly3 operator+(const Poly3& b) const
    {
        Poly3 ret;
        ret.a_ = a_ + b.a_;
        ret.b_ = b_ + b.b_;
        ret.c_ = c_ + b.c_;
        ret.d_ = d_ + b.d_;
        return ret;
    }

    /**
     * @brief Evaluates the polynomial for the given input value.
     *
     * @param t             The input value.
     * @returns             The value of f(t).
     */
    double eval(double t) const { return a_ + t * (b_ + t * (c_ + t * d_)); }

    /**
     * @brief Evaluates the derivative of the polynomial at the given input value.
     *
     * @param t             The input value.
     * @returns             The derivative f'(t).
     */
    double evalDerivative(double t) const { return b_ + t * (2 * c_ + t * 3 * d_); }

    /**
     * @brief Evaluates the second derivative of the polynomial at the given
     * input value.
     *
     * @param t             The input value.
     * @returns             The second derivative f''(t).
     */
    double eval2ndDerivative(double t) const { return 2 * c_ + t * 6 * d_; }

    /**
     * @brief Evaluates the anti derivative of the polynomial at the given input value.
     *
     * @param t             The input value
     * @returns             The anti-derivative evaluate at t.
     */
    double evalAntiDerivative(double t) const
    {
        return t * (a_ + t * ((1.0 / 2) * b_ + t * ((1.0 / 3) * c_ + t * (1.0 / 4) * d_)));
    }

    /**
     * @brief Finds the maximum value of the polynomial in the given interval.
     *
     * @param startT        The start of the interval
     * @param endT          The end of the interval
     * @return              The maximum value this polynomial takes in [startT, endT]
     */
    double maxValueInInterval(double startT, double endT) const;

    /**
     * @brief Finds the minimum value of the polynomial in the given interval.
     *
     * @param startT        The start of the interval
     * @param endT          The end of the interval
     * @return              The minimum value this polynomial takes in [startT, endT]
     */
    double minValueInInterval(double startT, double endT) const;

    /**
     * @brief Computes a Poly3 p, such that for any t, p.eval(t) == eval(t + offset) (barring any error introduced by
     * floating point math).
     *
     * @param offset        The translation offset
     * @return              The translated polynomial
     */
    Poly3 translate(const double offset) const;

    /**
     * @brief Computes a Poly3 p, such that for any t, p.eval(t) == eval(t * factor) (barring any error introduced by
     * floating point math).
     *
     * @param factor        The scaling factor
     * @return              The scaled polynomial
     */
    Poly3 scale(const double factor) const;

    /**
     * @brief The coefficient of the constant term.
     */
    double a_;

    /**
     * @brief The coefficient of the 1st degree term.
     */
    double b_;

    /**
     * @brief The coefficient of the 2nd degree term.
     */
    double c_;

    /**
     * @brief The coefficient of the 3rd degree term.
     */
    double d_;
};

}}  // namespace aid::xodr