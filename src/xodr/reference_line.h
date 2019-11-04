#pragma once

#include <memory>
#include <Eigen/Dense>

#include "xodr_reader.h"
#include "poly3.h"

namespace aid { namespace xodr {

class TestFactory;

/**
 * @brief The reference line of a road.
 *
 * The reference line of a road specifies its basic geometry and is the basis
 * of the st-coordinate system which is used for lane geometry.
 * The s-coordinate of an st-coordinate pair specifies the position along this
 * curve of the point, the t-coordinate the subsequent offset in the direction
 * parallel to the curve (left is positive).
 */
class ReferenceLine
{
    friend class TestFactory;

  public:
    struct Vertex
    {
        /**
         * @brief The s-coordinate of this vertex, measured from the beginning
         * of the reference line.
         */
        double sCoord_;

        /**
         * @brief The position of this vertex.
         */
        Eigen::Vector2d position_;

        /**
         * @brief The heading of this vertex.
         */
        double heading_;
    };

    /**
     * @brief A pair of a position and a tangent direction.
     *
     * This is used to represent points on the reference line, where the
     * tangent dir is the tangent direction of the reference line at that point.
     */
    struct PointAndTangentDir
    {
        PointAndTangentDir() = default;

        /**
         * @brief Creates a PointAndTangentDir with the given point and tangent direction.
         *
         * @param point         The point.
         * @param tangentDir    The tangent direction.
         */
        PointAndTangentDir(const Eigen::Vector2d& point, const Eigen::Vector2d& tangentDir)
            : point_(point), tangentDir_(tangentDir)
        {
        }

        /**
         * @brief Gets a vector pointing towards the side.
         *
         * The side dir is the vector obtained by rotating the tangent dir 90
         * degrees in the counter-clockwise direction.
         *
         * @return The side dir.
         */
        Eigen::Vector2d sideDir() const { return Eigen::Vector2d(-tangentDir_.y(), tangentDir_.x()); }

        /**
         * @brief Gets the point with the given t-coordinate, relative to this
         * point and tangent dir.
         *
         * The resulting point is obtained by adding an offset of t in the
         * direction of sideDir() to this pair's point.
         *
         * @param t         The t-coordinate.
         * @return          The point with the given t-coordinate.
         */
        Eigen::Vector2d pointWithTCoord(double t) const { return point_ + t * sideDir(); }

        /**
         * @brief The position.
         */
        Eigen::Vector2d point_;

        /**
         * @brief The tangent direction.
         */
        Eigen::Vector2d tangentDir_;
    };

    /**
     * @brief A Tessellation is a piecewise linear approximation of a reference line.
     */
    using Tessellation = std::vector<Vertex>;

    /**
     * @brief The type of a @ref Geometry.
     *
     * This is the return type of the  @ref Geometry::geometryType() and
     * indicates of which specific subtype the Geometry object is.
     */
    enum class GeometryType
    {
        /**
         * @brief The geometry is of type @ref Line.
         */
        LINE,

        /**
         * @brief The geometry is of type @ref Spiral.
         */
        SPIRAL,

        /**
         * @brief The geometry is of type @ref Arc.
         */
        ARC,

        /**
         * @brief The geometry is of type @ref Poly3.
         */
        POLY3,

        /**
         * @brief The geometry is of type @ref ParamPoly3.
         */
        PARAM_POLY3,
    };

    /**
     * @brief A struct which holds the values of the attributes of a geometry
     * element in a xodr file.
     *
     * This struct is only used as a temporary storage for this data while
     * parsing, and will eventually be stored in the @ref Geometry object.
     * The reason why we need this struct is because this data occurs in the
     * xodr file at a point when we don't yet know which specific type the
     * geometry is going to be (ie. Line, Spiral, etc.).
     */
    struct GeometryAttribs
    {
        Vertex startVertex_;
        double length_;

        /**
         * @brief Sets the sCoord_ member of startVertex_.
         */
        void setS(double s) { startVertex_.sCoord_ = s; }

        /**
         * @brief Sets the x coordinate of the position_ member of startVertex_.
         */
        void setX(double x) { startVertex_.position_.x() = x; }

        /**
         * @brief Sets the y coordinate of the position_ member of startVertex_.
         */
        void setY(double y) { startVertex_.position_.y() = y; }

        /**
         * @brief Sets the heading of startVertex_.
         */
        void setHeading(double heading) { startVertex_.heading_ = heading; }

        class AttribParsers;
    };

    /**
     * @brief A base class for the various curves which can be used to describe
     * the shape of a reference line.
     */
    class Geometry
    {
        friend class TestFactory;

      public:
        Geometry() = default;

        /**
         * @brief Construct the common part of a geometry object.
         *
         * @param startVertex  The start vertex.
         * @param length       The length of this part of the line.
         */
        Geometry(const Vertex& startVertex, double length);

        virtual ~Geometry() = default;

        /**
         * @brief Sets the values from the attributes coming from the <geometry>
         * xml element.
         *
         * @param geomAttribs  The geometry attributes.
         */
        void setGeometryAttribs(const GeometryAttribs& geomAttribs)
        {
            startVertex_ = geomAttribs.startVertex_;
            length_ = geomAttribs.length_;
        }

        /**
         * @brief Clone the geometry object.
         *
         * This is used to create a copy of a reference line, which in
         * turn is needed for the python bindings.
         */
        virtual Geometry* clone() const = 0;

        /**
         * @brief Returns the type of this geometry.
         *
         * @return      The geometry type.
         */
        virtual GeometryType geometryType() const = 0;

        /**
         * @brief Evaluates the point on this geometry with the given
         * s-coordinate and returns the point and tangent direction at that point.
         *
         * The s-coordinate must lie in the s-interval of this geometry (that is,
         * the interval [startVertex().sCoord_, endVertex().sCoord_).
         *
         * @param s     The s-coordinate.
         * @return      The resulting PointAndTangentDir.
         */
        virtual PointAndTangentDir eval(double s) const = 0;

        /**
         * @brief Evaluates the (signed) curvature on this geometry at the given
         * s-coordinate.
         *
         * The absolute value of curvature is the inverse of the radius of
         * curvature, and the sign of curvature is positive if the curvature is
         * directed counterclockwise.
         *
         * The s-coordinate must lie in the s-interval of this geometry (that is,
         * the interval [startVertex().sCoord_, endVertex().sCoord_).
         *
         * @param s     The s-coordinate.
         * @return      The signed curvature.
         */
        virtual double evalCurvature(double s) const = 0;

        /**
         * Tessellates the section of this geometry which falls in the
         * [startS, endS] range. [startS, endS] must be a subset of the full
         * range of this geometry and should not be empty (so
         * startVertex_.sCoord <= startS < endS <= startVertex_.sCoord + length_).
         *
         * The output will be appended to any existing data in @p tessellation.
         *
         * If @p includeEndPt is true, then the end point of the last line
         * segment of this geometry's tessellation should be appended, if
         * @p includeEndPt is false, then it should be omitted.
         */
        virtual void tessellate(Tessellation& tessellation, double startS, double endS, bool includeEndPt) const = 0;

        /**
         * @brief Gets the start vertex of this geometry.
         *
         * Note that the start vertex is stored inside this object, so this
         * function is the preferred way of getting the start vertex. In this
         * way, this function differs from the @ref endVertex function, which
         * generally shouldn't be used. See that function for more details.
         *
         * @return          The start vertex of this geometry/
         */
        Vertex startVertex() const { return startVertex_; }

        /**
         * @brief Computes the end vertex of this geometry.
         *
         * There's usually no need to call this function, because the end vertex
         * of a geometry is equal to the start vertex of the next one, or to
         * ChordLine::endVertex_ when it's the last geometry.
         *
         * The main purpose of this function is to be used while building
         * constructing ReferenceLine (as in the ReferenceLineBuilder), when
         * the next geometry or endVertex_ isn't available yet.
         *
         * @returns         The computed endVertex of this Geometry.
         */
        virtual Vertex endVertex() const = 0;

        /**
         * @brief Gets the length of this geometry.
         *
         * @return          The length of the geometry.
         */
        double length() const { return length_; }

      protected:
        /**
         * @brief Checks whether the given s-coordinate lies in the s-range of
         * the geometry.
         *
         * The check is done using a tolerance value, such that s-coordinates
         * which lie slightly outside the s-range are considered to lie inside it.
         *
         * @param s         The s-coordinate to check.
         * @return          True if the the given s-coord lies within this
         *                  geometry's s-range, false otherwise.
         */
        bool inSRange(double s) const;

      private:
        Vertex startVertex_;
        double length_;
    };

    /**
     * @brief A straight line geometry.
     */
    class Line : public Geometry
    {
      public:
        Line() = default;

        /**
         * @brief Constructs a line with the given 'from' and 'to' points.
         *
         * @param startS    The s-coordinate of the beginning of the line.
         * @param from      The start point of the line.
         * @param to        The end point of the line.
         */
        Line(double startS, const Eigen::Vector2d& from, const Eigen::Vector2d& to);

        /**
         * @brief Constructs a line.
         *
         * @param startVertex  The start vertex.
         * @param length       The length of this part of the line.
         */
        Line(const Vertex& startVertex, double length);

        /**
         * @brief Parses a Line using the given XodrReader.
         *
         * This function has @ref xml_parsers::parseXmlElem semantics.
         *
         * @param xml       The XodrReader.
         * @returns         The resulting Line.
         */
        static XodrParseResult<Line> parseXml(const GeometryAttribs& geomAttribs, XodrReader& xml);

        /**
         * @brief Clone the line.
         */
        virtual Line* clone() const override;

        /**
         * @brief The Line implementation of the geometryType() function.
         *
         * @returns GeometryType::LINE.
         */
        virtual GeometryType geometryType() const override { return GeometryType::LINE; }

        /**
         * @brief The Line implementation of the eval() function.
         *
         * See Geometry::eval() for more details.
         */
        virtual PointAndTangentDir eval(double s) const override;

        /**
         * @brief The Line implementation of the evalCurvature() function.
         *
         * See Geometry::eval() for more details.
         */
        virtual double evalCurvature(double s) const override;

        /**
         * @brief The Line implementation of the tessellate function.
         *
         * See Geometry::tessellate() for more details.
         */
        virtual void tessellate(Tessellation& tessellation, double startS, double endS,
                                bool includeEndPt) const override;

        /**
         * @brief The Line implementation of the endVertex function.
         *
         * See Geometry::endVertex() for more details.
         */
        virtual Vertex endVertex() const override;
    };

    /**
     * @brief A Euler spiral geometry.
     *
     * This is the curve whose curvature changes linearly from the spiral's
     * start curvature to the spiral's end curvature with the length of the curve.
     */
    class Spiral : public Geometry
    {
      public:
        Spiral() = default;

        /**
         * Constructs a Spiral instance using the given GeometryAttribs and
         * start and end curvature.
         *
         * @param geomAttribs       The GeometryAttribs
         * @param startCurvature    The start curvature.
         * @param endCurvature      The end curvature.
         */
        Spiral(const GeometryAttribs& geomAttribs, double startCurvature, double endCurvature);

        /**
         * @brief Constructs a spiral.
         *
         * @param startVertex  The start vertex.
         * @param length       The length of this part of the line.
         * @param startCurvature    The start curvature.
         * @param endCurvature      The end curvature.
         */
        Spiral(const Vertex& startVertex, double length, double startCurvature, double endCurvature);

        /**
         * @brief Parses a Spiral using the given XodrReader.
         *
         * This function has @ref xml_parsers::parseXmlElem semantics.
         *
         * @param xml       The XodrReader.
         * @returns         The resulting Spiral.
         */
        static XodrParseResult<Spiral> parseXml(const GeometryAttribs& geomAttribs, XodrReader& xml);

        /**
         * @brief Clone the spiral.
         */
        virtual Spiral* clone() const override;

        /**
         * @brief The Spiral implementation of the geometryType() function.
         *
         * @returns GeometryType::SPIRAL.
         */
        virtual GeometryType geometryType() const override { return GeometryType::SPIRAL; }

        /**
         * @brief The Spiral implementation of the eval() function.
         *
         * See Geometry::eval() for more details.
         */
        virtual PointAndTangentDir eval(double s) const override;

        /**
         * @brief The Spiral implementation of the evalCurvature() function.
         *
         * See Geometry::eval() for more details.
         */
        virtual double evalCurvature(double s) const override;

        /**
         * @brief The Spiral implementation of the tessellate function.
         *
         * See @ref Geometry::Tessellate for more details.
         */
        virtual void tessellate(Tessellation& tessellation, double startS, double endS,
                                bool includeEndPt) const override;

        /**
         * @brief The Spiral implementation of the endVertex function.
         *
         * See Geometry::endVertex() for more details.
         */
        virtual Vertex endVertex() const override;

        double startCurvature() const { return startCurvature_; }
        double endCurvature() const { return endCurvature_; }
        double curvatureRateOfChange() const { return (endCurvature_ - startCurvature_) / length(); }

      private:
        class AttribParsers;

        double startCurvature_;
        double endCurvature_;
    };

    /**
     * @brief An arc geometry.
     *
     * This is a curve with constant curvature, so it's a segment of a circle.
     */
    class Arc : public Geometry
    {
      public:
        Arc() = default;

        /**
         * @brief Constructs a arc.
         *
         * @param startVertex  The start vertex.
         * @param length       The length of this part of the line.
         * @param curvature    The curvature of this arc.
         */
        Arc(const Vertex& startVertex, double length, double curvature);

        /**
         * @brief Constructs an arc from a circle segment. The circle is
         * specified using a center and radius, the segment using a start angle
         * and angular range.
         *
         * @param startS        The start s-coordinate of the arc.
         * @param circleCenter  The center of the circle.
         * @param radius        The radius of the circle.
         * @param startAngle    The angle of the start point on the circle. The
         *                      angle is relative to the positive x-axis.
         * @param segmentAngle  The central angle of the segment. Use a positive
         *                      angle for an arc which extends in the counter-
         *                      clockwise direction from the start point, and
         *                      a negative angle for an arc in the clockwise direction.
         * @return The Arc constructed from the given parameters.
         */
        static Arc fromCircleSegment(double startS, const Eigen::Vector2d& circleCenter, double radius,
                                     double startAngle, double segmentAngle);

        /**
         * @brief Parses an Arc using the given XodrReader.
         *
         * This function has @ref xml_parsers::parseXmlElem semantics.
         *
         * @param xml       The XodrReader.
         * @returns         The resulting Arc.
         */
        static XodrParseResult<Arc> parseXml(const GeometryAttribs& geomAttribs, XodrReader& xml);

        /**
         * @brief Clone the arc.
         */
        virtual Arc* clone() const override;

        /**
         * @brief The Arc implementation of the geometryType() function.
         *
         * @returns GeometryType::ARC.
         */
        virtual GeometryType geometryType() const override { return GeometryType::ARC; }

        /**
         * @brief The Arc implementation of the eval() function.
         *
         * See Geometry::eval() for more details.
         */
        virtual PointAndTangentDir eval(double s) const override;

        /**
         * @brief The Arc implementation of the evalCurvature() function.
         *
         * See Geometry::eval() for more details.
         */
        virtual double evalCurvature(double s) const override;

        /**
         * @brief The Arc implementation of the tessellate function.
         *
         * See @ref Geometry::Tessellate for more details.
         */
        virtual void tessellate(Tessellation& tessellation, double startS, double endS,
                                bool includeEndPt) const override;

        /**
         * @brief The Arc implementation of the endVertex function.
         *
         * See Geometry::endVertex() for more details.
         */
        virtual Vertex endVertex() const override;

        double curvature() const { return curvature_; }

      private:
        class AttribParsers;

        double curvature_;
    };

    /**
     * @brief A cubic polynomial geometry.
     *
     * A Poly3Geom describes the curve (x, f(x)) in a local coordinate system.
     * To go from this coordinate system to the road's coordinate system, the
     * origin is translated to the geometry's start vertex, and the local
     * coordinate system's x coordinate is rotated to match the start vertex' heading.
     */
    class Poly3Geom : public Geometry
    {
        friend class TestFactory;

      public:
        Poly3Geom() = default;

        /**
         * @brief Constructs a Poly3Geom using the given parameters.
         *
         * @param geomAttribs   The GeometryAttribs.
         * @param poly          The polynomial.
         */
        Poly3Geom(const GeometryAttribs& geomAttribs, const Poly3& poly);

        /**
         * @brief Constructs a polynomial.
         *
         * @param startVertex  The start vertex.
         * @param length       The length of this part of the line.
         * @param poly         The polynomial.
         */
        Poly3Geom(const Vertex& startVertex, double length, const Poly3& poly);

        /**
         * @brief Parses a Poly3Geom using the given XodrReader.
         *
         * This function has @ref xml_parsers::parseXmlElem semantics.
         *
         * @param xml       The XodrReader.
         * @returns         The resulting Poly3Geom.
         */
        static XodrParseResult<Poly3Geom> parseXml(const GeometryAttribs& geomAttribs, XodrReader& xml);

        /**
         * @brief Clone the polynomial.
         */
        virtual Poly3Geom* clone() const override;

        /**
         * @brief The Poly3Geom implementation of the geometryType() function.
         *
         * @returns GeometryType::POLY3.
         */
        virtual GeometryType geometryType() const override { return GeometryType::POLY3; }

        /**
         * @brief The Poly3Geom implementation of the eval() function.
         *
         * See Geometry::eval() for more details.
         */
        virtual PointAndTangentDir eval(double s) const override;

        /**
         * @brief The Poly3Geom implementation of the evalCurvature() function.
         *
         * See Geometry::eval() for more details.
         */
        virtual double evalCurvature(double s) const override;

        /**
         * @brief The Poly3Geom implementation of the tessellate function.
         *
         * See @ref Geometry::Tessellate for more details.
         */
        virtual void tessellate(Tessellation& tessellation, double startS, double endS,
                                bool includeEndPt) const override;

        /**
         * @brief The Poly3Geom implementation of the endVertex function.
         *
         * See Geometry::endVertex() for more details.
         */
        virtual Vertex endVertex() const override;

        double endCurvature() const;

        const Poly3& poly() const { return poly_; }

      private:
        class AttribParsers;

        void setA(double a) { poly_.a_ = a; }
        void setB(double b) { poly_.b_ = b; }
        void setC(double c) { poly_.c_ = c; }
        void setD(double d) { poly_.d_ = d; }

        Poly3 poly_;
    };

    /**
     * @brief The parameter ranges for the ParamPoly3 geometry.
     *
     * This specifies the range of the input values used to evaluate the two
     * polynomials in a ParamPoly3.
     */
    enum class PRange
    {
        /**
         * The parameter ranges from 0 at the start vertex to ParamPoly3::length()
         * at the end vertex.
         */
        ARC_LENGTH,

        /**
         * The parameter ranges from 0 at the start vertex to 1 at the end vertex.
         */
        NORMALIZED
    };

    /**
     * @brief A parametric cubic polynomial geometry.
     *
     * A ParamPoly3 describes the curve (uPoly(t), vPoly(t)) in a local
     * coordinate system. To go from this coordinate system to the road's
     * coordinate system, the origin is translated to the geometry's start
     * vertex, and the local coordinate system's x coordinate is rotated to
     * match the start vertex' heading.
     */
    class ParamPoly3 : public Geometry
    {
        friend class TestFactory;

      public:
        ParamPoly3() = default;

        /**
         * Constructs a ParamPoly3 using the given parameters.
         *
         * @param geomAttribs   The GeometryAttribs.
         * @param uPoly         The polynomial for the u-coordinate.
         * @param vPoly         The polynomial for the v-coordinate.
         * @param pRange        The parameter range.
         */
        ParamPoly3(const GeometryAttribs& geomAttribs, const Poly3& uPoly, const Poly3& vPoly, PRange pRange);

        /**
         * @brief Constructs a polynomial.
         *
         * @param startVertex  The start vertex.
         * @param length       The length of this part of the line.
         * @param uPoly        The polynomial for the u-coordinate.
         * @param vPoly        The polynomial for the v-coordinate.
         * @param pRange       The parameter range.
         */
        ParamPoly3(const Vertex& startVertex, double length, const Poly3& uPoly, const Poly3& vPoly, PRange pRange);

        /**
         * @brief Parses a ParamPoly3 using the given XodrReader.
         *
         * This function has @ref xml_parsers::parseXmlElem semantics.
         *
         * @param xml       The XodrReader.
         * @returns         The resulting ParamPoly3.
         */
        static XodrParseResult<ParamPoly3> parseXml(const GeometryAttribs& geomAttribs, XodrReader& xml);

        /**
         * @brief Clone the polynomial.
         */
        virtual ParamPoly3* clone() const override;

        /**
         * @brief The ParamPoly3 implementation of the geometryType() function.
         *
         * @returns GeometryType::PARAM_POLY3.
         */
        virtual GeometryType geometryType() const override { return GeometryType::PARAM_POLY3; }

        /**
         * @brief The ParamPoly3 implementation of the eval() function.
         *
         * See Geometry::eval() for more details.
         */
        virtual PointAndTangentDir eval(double s) const override;

        /**
         * @brief The ParamPoly3 implementation of the evalCurvature() function.
         *
         * See Geometry::eval() for more details.
         */
        virtual double evalCurvature(double s) const override;

        /**
         * @brief The ParamPoly3 implementation of the tessellate function.
         *
         * See @ref Geometry::Tessellate for more details.
         */
        virtual void tessellate(Tessellation& tessellation, double startS, double endS,
                                bool includeEndPt) const override;

        /**
         * @brief The Line implementation of the endVertex function.
         *
         * See Geometry::endVertex() for more details.
         */
        virtual Vertex endVertex() const override;

        /**
         * @return The u-polynomial of this ParamPoly3.
         */
        const Poly3& uPoly() const { return uPoly_; }

        /**
         * @return The v-polynomial of this ParamPoly3.
         */
        const Poly3& vPoly() const { return vPoly_; }

        /**
         * @return The parameter range of this param poly.
         *
         * See @ref PRange for a description of the different ranges.
         */
        PRange pRange() const { return pRange_; }

      private:
        class AttribParsers;

        void setAU(double au) { uPoly_.a_ = au; }
        void setBU(double bu) { uPoly_.b_ = bu; }
        void setCU(double cu) { uPoly_.c_ = cu; }
        void setDU(double du) { uPoly_.d_ = du; }

        void setAV(double av) { vPoly_.a_ = av; }
        void setBV(double bv) { vPoly_.b_ = bv; }
        void setCV(double cv) { vPoly_.c_ = cv; }
        void setDV(double dv) { vPoly_.d_ = dv; }

        Poly3 uPoly_;
        Poly3 vPoly_;
        PRange pRange_;
    };

    /**
     * @brief Creates an empty reference line.
     */
    ReferenceLine() = default;

    /**
     * @brief Copy a reference line.
     */
    ReferenceLine(const ReferenceLine& referenceLine);
    ReferenceLine& operator=(const ReferenceLine& referenceLine);

    /**
     * @brief Parses a ReferenceLine from a <planView> xodr element
     * out of the given text.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param text      The text data to read.
     * @returns         The resulting ReferenceLine.
     */
    static XodrParseResult<ReferenceLine> fromText(const std::string& text);

    /**
     * @brief Parses a ReferenceLine from a <planView> xodr element using the
     * given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting ReferenceLine.
     */
    static XodrParseResult<ReferenceLine> parseXml(XodrReader& xml);

    /**
     * @brief Evaluates the point on the reference line with the given
     * s-coordinate and returns the point and tangent direction at that point.
     *
     * The s-coordinate must lie in the s-interval of the reference line (that is,
     * the interval [0, endS())).
     *
     * @param s     The s-coordinate.
     * @return      The resulting PointAndTangentDir.
     */
    PointAndTangentDir eval(double s) const;

    /**
     * @brief Evaluates the (signed) curvature on this geometry at the given
     * s-coordinate.
     *
     * The absolute value of curvature is the inverse of the radius of
     * curvature, and the sign of curvature is positive if the curvature is
     * directed counterclockwise.
     *
     * The s-coordinate must lie in the s-interval of this geometry (that is,
     * the interval [startVertex().sCoord_, endVertex().sCoord_).
     *
     * @param s     The s-coordinate.
     * @return      The signed curvature.
     */
    double evalCurvature(double s) const;

    /**
     * Returns a piecewise linear approximation of the section of this chord
     * line with s values in the interval [startS, endS].
     */
    Tessellation tessellate(double startS, double endS) const;

    /**
     * Returns the end s coordinate of this chord line.
     *
     * Note that there's no startS() function, because the start s coordinate
     * is always 0.
     */
    double endS() const { return endVertex_.sCoord_; }

    /**
     * @returns The end vertex of the reference line.
     */
    const Vertex& endVertex() const { return endVertex_; }

    /**
     * @brief Gets the number of geometries in this reference line.
     */
    int numGeometries() const { return static_cast<int>(geometries_.size()); }

    /**
     * @brief Gets the geometry with the given index.
     */
    const Geometry& geometry(int i) const { return *geometries_[i]; }

  private:
    const Geometry& geometryContaining(double s) const;

    std::vector<std::unique_ptr<Geometry>> geometries_;
    Vertex endVertex_;
};

}}  // namespace aid::xodr
