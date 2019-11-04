#include "reference_line.h"

#include <cmath>

extern "C" {
#include "odrSpiral/odrSpiral.h"
}

namespace aid { namespace xodr {

static const double NUM_VERTICES_PER_METER = 1;

ReferenceLine::ReferenceLine(const ReferenceLine& referenceLine) : endVertex_(referenceLine.endVertex_)
{
    geometries_.reserve(referenceLine.geometries_.size());
    for (const auto& geometry : referenceLine.geometries_)
    {
        geometries_.emplace_back(geometry->clone());
    }
}

ReferenceLine& ReferenceLine::operator=(const ReferenceLine& referenceLine)
{
    endVertex_ = referenceLine.endVertex_;
    geometries_.clear();
    geometries_.reserve(referenceLine.geometries_.size());
    for (const auto& geometry : referenceLine.geometries_)
    {
        geometries_.emplace_back(geometry->clone());
    }
    return *this;
}

const ReferenceLine::Geometry& ReferenceLine::geometryContaining(double s) const
{
    assert(s >= -.00001 && s <= endVertex_.sCoord_ + .00001);

    int geomsMin = 0;
    int geomsMax = static_cast<int>(geometries_.size());

    while (geomsMin != geomsMax - 1)
    {
        int mid = (geomsMin + geomsMax) / 2;
        if (s < geometries_[mid]->startVertex().sCoord_)
        {
            geomsMax = mid;
        }
        else
        {
            geomsMin = mid;
        }
    }
    return *geometries_[geomsMin];
}

ReferenceLine::PointAndTangentDir ReferenceLine::eval(double s) const
{
    return geometryContaining(s).eval(s);
}

double ReferenceLine::evalCurvature(double s) const
{
    return geometryContaining(s).evalCurvature(s);
}

ReferenceLine::Tessellation ReferenceLine::tessellate(double startS, double endS) const
{
    assert(!geometries_.empty());
    assert(startS >= geometries_[0]->startVertex().sCoord_);
    assert(endS <= endVertex_.sCoord_);
    assert(startS < endS);

    Tessellation ret;

    for (int i = 0; i < static_cast<int>(geometries_.size()); i++)
    {
        const Geometry& geom = *geometries_[i];

        double geomStartS = geom.startVertex().sCoord_;
        double geomEndS;
        if (i == static_cast<int>(geometries_.size()) - 1)
        {
            geomEndS = geomStartS + geom.length();
        }
        else
        {
            geomEndS = geometries_[i + 1]->startVertex().sCoord_;
        }

        double clampedStartS = std::max(startS, geomStartS);
        double clampedEndS = std::min(endS, geomEndS);
        if (clampedStartS < clampedEndS)
        {
            geom.tessellate(ret, clampedStartS, clampedEndS, clampedEndS == endS);
        }
    }

    return ret;
}

ReferenceLine::Geometry::Geometry(const Vertex& startVertex, double length) : startVertex_(startVertex), length_(length)
{
}

bool ReferenceLine::Geometry::inSRange(double s) const
{
    double localS = s - startVertex_.sCoord_;
    return localS >= -.00001 && localS < length_ + .00001;
}

ReferenceLine::Line::Line(double startS, const Eigen::Vector2d& from, const Eigen::Vector2d& to)
{
    assert(!from.isApprox(to));

    Eigen::Vector2d dir = to - from;

    GeometryAttribs geomAttribs;
    geomAttribs.startVertex_.sCoord_ = startS;
    geomAttribs.startVertex_.position_ = from;
    geomAttribs.startVertex_.heading_ = std::atan2(dir.y(), dir.x());
    geomAttribs.length_ = dir.norm();
    setGeometryAttribs(geomAttribs);
}

ReferenceLine::Line::Line(const Vertex& startVertex, double length) : Geometry(startVertex, length) {}

ReferenceLine::Line* ReferenceLine::Line::clone() const
{
    return new Line(startVertex(), length());
}

ReferenceLine::PointAndTangentDir ReferenceLine::Line::eval(double s) const
{
    const Vertex& startVert = startVertex();

    assert(s >= startVert.sCoord_ && s <= startVert.sCoord_ + length());

    PointAndTangentDir ret;
    ret.tangentDir_ = Eigen::Vector2d(std::cos(startVert.heading_), std::sin(startVert.heading_));
    ret.point_ = startVert.position_ + (s - startVert.sCoord_) * ret.tangentDir_;
    return ret;
}

double ReferenceLine::Line::evalCurvature(double s) const
{
    assert(inSRange(s));
    (void)s;
    return 0;
}

void ReferenceLine::Line::tessellate(Tessellation& tessellation, double startS, double endS, bool includeEndPt) const
{
    const Vertex& startVert = startVertex();

    assert(startS >= startVert.sCoord_);
    assert(endS <= startVert.sCoord_ + length() + .00001);
    assert(startS < endS);

    Eigen::Vector2d forward(std::cos(startVert.heading_), std::sin(startVert.heading_));

    double startT = startS - startVert.sCoord_;

    int num = static_cast<int>(std::ceil((endS - startS) * NUM_VERTICES_PER_METER));
    double stepSize = (endS - startS) / num;

    if (includeEndPt)
    {
        num++;
    }

    for (int i = 0; i < num; i++)
    {
        double t = startT + i * stepSize;

        Vertex vert;
        vert.sCoord_ = startS + i * stepSize;
        vert.position_ = startVert.position_ + t * forward;
        vert.heading_ = startVert.heading_;
        tessellation.push_back(vert);
    }
}

ReferenceLine::Vertex ReferenceLine::Line::endVertex() const
{
    const Vertex& startVert = startVertex();

    Eigen::Vector2d forward(std::cos(startVert.heading_), std::sin(startVert.heading_));

    Vertex ret;
    ret.sCoord_ = startVert.sCoord_ + length();
    ret.position_ = startVert.position_ + length() * forward;
    ret.heading_ = startVert.heading_;
    return ret;
}

ReferenceLine::Spiral::Spiral(const GeometryAttribs& geomAttribs, double startCurvature, double endCurvature)
{
    assert(startCurvature != endCurvature);

    setGeometryAttribs(geomAttribs);
    startCurvature_ = startCurvature;
    endCurvature_ = endCurvature;
}

ReferenceLine::Spiral::Spiral(const Vertex& startVertex, double length, double startCurvature, double endCurvature)
    : Geometry(startVertex, length), startCurvature_(startCurvature), endCurvature_(endCurvature)
{
}

ReferenceLine::Spiral* ReferenceLine::Spiral::clone() const
{
    return new Spiral(startVertex(), length(), startCurvature_, endCurvature_);
}

ReferenceLine::PointAndTangentDir ReferenceLine::Spiral::eval(double s) const
{
    assert(inSRange(s));

    const Vertex& startVert = startVertex();

    double curvatureROC = curvatureRateOfChange();
    double curveStartParam = startCurvature_ / curvatureROC;
    double curveEvalParam = curveStartParam + (s - startVert.sCoord_);

    Eigen::Vector2d curveStartPt, curveEndPt;
    double curveStartHeading, curveEvalHeading;

    odrSpiral(curveStartParam, curvatureROC, &curveStartPt.x(), &curveStartPt.y(), &curveStartHeading);
    odrSpiral(curveEvalParam, curvatureROC, &curveEndPt.x(), &curveEndPt.y(), &curveEvalHeading);

    Eigen::Vector2d offset = curveEndPt - curveStartPt;
    offset = Eigen::Rotation2Dd(startVert.heading_ - curveStartHeading) * offset;

    double heading = startVert.heading_ + (curveEvalHeading - curveStartHeading);

    PointAndTangentDir ret;
    ret.point_ = startVert.position_ + offset;
    ret.tangentDir_ = Eigen::Vector2d(std::cos(heading), std::sin(heading));
    return ret;
}

double ReferenceLine::Spiral::evalCurvature(double s) const
{
    assert(inSRange(s));

    return startCurvature_ + (s - startVertex().sCoord_) * curvatureRateOfChange();
}

void ReferenceLine::Spiral::tessellate(Tessellation& tessellation, double startS, double endS, bool includeEndPt) const
{
    const Vertex& startVert = startVertex();

    double rateOfChange = (endCurvature_ - startCurvature_) / length();
    double curveStartParam = startCurvature_ / rateOfChange;

    Eigen::Vector2d curveStartPt;
    double curveStartHeading;
    odrSpiral(curveStartParam, rateOfChange, &curveStartPt.x(), &curveStartPt.y(), &curveStartHeading);

    Eigen::Matrix2d rotation = Eigen::Rotation2Dd(startVert.heading_ - curveStartHeading).toRotationMatrix();

    int num = static_cast<int>(std::ceil((endS - startS) * NUM_VERTICES_PER_METER));
    double stepSize = (endS - startS) / num;

    if (includeEndPt)
    {
        num++;
    }

    double startParam = curveStartParam + (startS - startVert.sCoord_);
    for (int i = 0; i < num; i++)
    {
        Vertex vert;

        vert.sCoord_ = startS + i * stepSize;

        double t = startParam + i * stepSize;
        Eigen::Vector2d curvePt;
        double curveHeading;
        odrSpiral(t, rateOfChange, &curvePt.x(), &curvePt.y(), &curveHeading);

        vert.position_ = rotation * (curvePt - curveStartPt) + startVert.position_;
        vert.heading_ = startVert.heading_ + (curveHeading - curveStartHeading);

        tessellation.push_back(vert);
    }
}

ReferenceLine::Vertex ReferenceLine::Spiral::endVertex() const
{
    const Vertex& startVert = startVertex();

    double curvatureROC = curvatureRateOfChange();
    double curveStartParam = startCurvature_ / curvatureROC;
    double curveEndParam = curveStartParam + length();

    Eigen::Vector2d curveStartPt, curveEndPt;
    double curveStartHeading, curveEndHeading;

    odrSpiral(curveStartParam, curvatureROC, &curveStartPt.x(), &curveStartPt.y(), &curveStartHeading);
    odrSpiral(curveEndParam, curvatureROC, &curveEndPt.x(), &curveEndPt.y(), &curveEndHeading);

    Eigen::Vector2d offset = curveEndPt - curveStartPt;

    Eigen::Rotation2Dd rotation(startVert.heading_ - curveStartHeading);
    offset = rotation * offset;

    Vertex ret;
    ret.sCoord_ = startVert.sCoord_ + length();
    ret.position_ = startVert.position_ + offset;
    ret.heading_ = startVert.heading_ + (curveEndHeading - curveStartHeading);
    return ret;
}

ReferenceLine::Arc::Arc(const Vertex& startVertex, double length, double curvature)
    : Geometry(startVertex, length), curvature_(curvature)
{
}

ReferenceLine::Arc ReferenceLine::Arc::fromCircleSegment(double startS, const Eigen::Vector2d& circleCenter,
                                                         double radius, double startAngle, double segmentAngle)
{
    Eigen::Vector2d toStart(std::cos(startAngle), std::sin(startAngle));
    toStart *= radius;

    GeometryAttribs geomAttribs;
    geomAttribs.startVertex_.sCoord_ = startS;
    geomAttribs.startVertex_.position_ = circleCenter + toStart;

    Arc ret;

    if (segmentAngle > 0)
    {
        geomAttribs.startVertex_.heading_ = startAngle + .5 * M_PI;
        ret.curvature_ = 1 / radius;
    }
    else
    {
        geomAttribs.startVertex_.heading_ = startAngle - .5 * M_PI;
        ret.curvature_ = -1 / radius;
    }

    geomAttribs.length_ = std::abs(segmentAngle) * radius;
    ret.setGeometryAttribs(geomAttribs);

    return ret;
}

ReferenceLine::Arc* ReferenceLine::Arc::clone() const
{
    return new Arc(startVertex(), length(), curvature_);
}

ReferenceLine::PointAndTangentDir ReferenceLine::Arc::eval(double s) const
{
    assert(inSRange(s));

    const Vertex& startVert = startVertex();

    double radius = 1 / curvature_;

    Eigen::Vector2d toCenter(-std::sin(startVert.heading_), std::cos(startVert.heading_));
    Eigen::Vector2d center = startVert.position_ + toCenter * radius;

    double heading = startVert.heading_ + (s - startVert.sCoord_) * curvature_;

    PointAndTangentDir ret;
    ret.tangentDir_ = Eigen::Vector2d(std::cos(heading), std::sin(heading));
    ret.point_ = center + Eigen::Vector2d(ret.tangentDir_.y(), -ret.tangentDir_.x()) * radius;
    return ret;
}

double ReferenceLine::Arc::evalCurvature(double s) const
{
    assert(inSRange(s));
    (void)s;
    return curvature_;
}

void ReferenceLine::Arc::tessellate(Tessellation& tessellation, double startS, double endS, bool includeEndPt) const
{
    const Vertex& startVert = startVertex();

    double radius = 1 / curvature_;

    Eigen::Vector2d toCenter(-std::sin(startVert.heading_), std::cos(startVert.heading_));
    Eigen::Vector2d center = startVert.position_ + toCenter * radius;

    int num = static_cast<int>(std::ceil((endS - startS) * NUM_VERTICES_PER_METER));
    double stepSize = (endS - startS) / num;

    if (includeEndPt)
    {
        num++;
    }

    double clampedStartHeading = startVert.heading_ + (startS - startVert.sCoord_) * curvature_;
    for (int i = 0; i < num; i++)
    {
        Vertex vert;

        vert.sCoord_ = startS + i * stepSize;
        vert.heading_ = clampedStartHeading + i * stepSize * curvature_;

        Eigen::Vector2d toCircle(std::sin(vert.heading_), -std::cos(vert.heading_));
        vert.position_ = center + toCircle * radius;

        tessellation.push_back(vert);
    }
}

ReferenceLine::Vertex ReferenceLine::Arc::endVertex() const
{
    const Vertex& startVert = startVertex();

    double radius = 1 / curvature_;
    Eigen::Vector2d startNormal(-std::sin(startVert.heading_), std::cos(startVert.heading_));
    Eigen::Vector2d center = startVert.position_ + startNormal * radius;

    Vertex ret;
    ret.sCoord_ = startVert.sCoord_ + length();
    ret.heading_ = startVert.heading_ + length() * curvature_;

    Eigen::Vector2d endNormal(-std::sin(ret.heading_), std::cos(ret.heading_));
    ret.position_ = center - endNormal * radius;

    return ret;
}

ReferenceLine::Poly3Geom::Poly3Geom(const GeometryAttribs& geomAttribs, const Poly3& poly)
{
    setGeometryAttribs(geomAttribs);
    poly_ = poly;
}

ReferenceLine::Poly3Geom::Poly3Geom(const Vertex& startVertex, double length, const Poly3& poly)
    : Geometry(startVertex, length), poly_(poly)
{
}

ReferenceLine::Poly3Geom* ReferenceLine::Poly3Geom::clone() const
{
    return new Poly3Geom(startVertex(), length(), poly_);
}

ReferenceLine::PointAndTangentDir ReferenceLine::Poly3Geom::eval(double s) const
{
    assert(inSRange(s));

    const Vertex& startVert = startVertex();
    Eigen::Vector2d forward(std::cos(startVert.heading_), std::sin(startVert.heading_));
    Eigen::Vector2d side(-forward.y(), forward.x());

    PointAndTangentDir ret;

    double u = s - startVert.sCoord_;
    double v = poly_.eval(u);
    ret.point_ = startVert.position_ + u * forward + v * side;

    double du = 1;
    double dv = poly_.evalDerivative(u);
    ret.tangentDir_ = (du * forward + dv * side).normalized();

    return ret;
}

double ReferenceLine::Poly3Geom::evalCurvature(double s) const
{
    assert(inSRange(s));

    double u = s - startVertex().sCoord_;
    double derivative = poly_.evalDerivative(u);

    return poly_.eval2ndDerivative(u) / std::pow(1 + derivative * derivative, 1.5);
}

void ReferenceLine::Poly3Geom::tessellate(Tessellation& tessellation, double startS, double endS,
                                          bool includeEndPt) const
{
    const Vertex& startVert = startVertex();

    assert(startS >= startVert.sCoord_);
    assert(endS <= startVert.sCoord_ + length() + .00001);
    assert(startS < endS);

    Eigen::Vector2d forward(std::cos(startVert.heading_), std::sin(startVert.heading_));
    Eigen::Vector2d side(-forward.y(), forward.x());

    double startU = startS - startVert.sCoord_;

    int num = static_cast<int>(std::ceil((endS - startS) * NUM_VERTICES_PER_METER));
    double stepSize = (endS - startS) / num;

    if (includeEndPt)
    {
        num++;
    }

    for (int i = 0; i < num; i++)
    {
        Vertex vertex;
        vertex.sCoord_ = startS + i * stepSize;

        double u = startU + i * stepSize;
        double v = poly_.eval(u);
        vertex.position_ = startVert.position_ + u * forward + v * side;

        double headingDiff = std::atan(poly_.evalDerivative(u));
        vertex.heading_ = startVert.heading_ + headingDiff;

        tessellation.push_back(vertex);
    }
}

ReferenceLine::Vertex ReferenceLine::Poly3Geom::endVertex() const
{
    const Vertex& startVert = startVertex();

    Eigen::Vector2d forward(std::cos(startVert.heading_), std::sin(startVert.heading_));
    Eigen::Vector2d side(-forward.y(), forward.x());

    double endU = length();
    double endV = poly_.eval(length());

    double headingDiff = std::atan(poly_.evalDerivative(length()));

    Vertex ret;
    ret.sCoord_ = startVert.sCoord_ + endU;
    ret.position_ = startVert.position_ + endU * forward + endV * side;
    ret.heading_ = startVert.heading_ + headingDiff;
    return ret;
}

double ReferenceLine::Poly3Geom::endCurvature() const
{
    double vDeriv = poly_.evalDerivative(length());
    double v2ndDeriv = poly_.eval2ndDerivative(length());

    double sqrSpeed = 1 + vDeriv * vDeriv;
    double curvature = v2ndDeriv / (std::sqrt(sqrSpeed) * sqrSpeed);

    return curvature;
}

ReferenceLine::ParamPoly3::ParamPoly3(const GeometryAttribs& geomAttribs, const Poly3& uPoly, const Poly3& vPoly,
                                      PRange pRange)
{
    setGeometryAttribs(geomAttribs);
    uPoly_ = uPoly;
    vPoly_ = vPoly;
    pRange_ = pRange;
}

ReferenceLine::ParamPoly3::ParamPoly3(const Vertex& startVertex, double length, const Poly3& uPoly, const Poly3& vPoly,
                                      PRange pRange)
    : Geometry(startVertex, length), uPoly_(uPoly), vPoly_(vPoly), pRange_(pRange)
{
}

ReferenceLine::ParamPoly3* ReferenceLine::ParamPoly3::clone() const
{
    return new ParamPoly3(startVertex(), length(), uPoly_, vPoly_, pRange_);
}

ReferenceLine::PointAndTangentDir ReferenceLine::ParamPoly3::eval(double s) const
{
    assert(inSRange(s));

    const Vertex& startVert = startVertex();
    Eigen::Vector2d forward(std::cos(startVert.heading_), std::sin(startVert.heading_));
    Eigen::Vector2d side(-forward.y(), forward.x());

    PointAndTangentDir ret;

    double param = s - startVert.sCoord_;
    if (pRange_ == PRange::NORMALIZED)
    {
        param /= length();
    }

    double u = uPoly_.eval(param);
    double v = vPoly_.eval(param);
    ret.point_ = startVert.position_ + u * forward + v * side;

    double du = uPoly_.evalDerivative(param);
    double dv = vPoly_.evalDerivative(param);
    ret.tangentDir_ = (du * forward + dv * side).normalized();

    return ret;
}

double ReferenceLine::ParamPoly3::evalCurvature(double s) const
{
    assert(inSRange(s));

    double param = s - startVertex().sCoord_;
    if (pRange_ == PRange::NORMALIZED)
    {
        param /= length();
    }
    double numerator = uPoly_.evalDerivative(param) * vPoly_.eval2ndDerivative(param) -
                       vPoly_.evalDerivative(param) * uPoly_.eval2ndDerivative(param);
    double derivativeU = uPoly_.evalDerivative(param);
    double derivativeV = vPoly_.evalDerivative(param);
    double denominator = std::pow(derivativeU * derivativeU + derivativeV * derivativeV, 1.5);
    return numerator / denominator;
}

void ReferenceLine::ParamPoly3::tessellate(Tessellation& tessellation, double startS, double endS,
                                           bool includeEndPt) const
{
    const Vertex& startVert = startVertex();

    assert(startS >= startVert.sCoord_);
    assert(endS <= startVert.sCoord_ + length() + .00001);
    assert(startS < endS);

    Eigen::Vector2d forward(std::cos(startVert.heading_), std::sin(startVert.heading_));
    Eigen::Vector2d side(-forward.y(), forward.x());

    double startParam = startS - startVert.sCoord_;

    int num = static_cast<int>(std::ceil((endS - startS) * NUM_VERTICES_PER_METER));

    double stepSize = (endS - startS) / num;
    double paramStepSize = stepSize;

    if (pRange_ == PRange::NORMALIZED)
    {
        double scale = 1 / length();
        startParam *= scale;
        paramStepSize *= scale;
    }

    if (includeEndPt)
    {
        num++;
    }

    for (int i = 0; i < num; i++)
    {
        Vertex vert;

        vert.sCoord_ = startS + i * stepSize;

        double t = startParam + i * paramStepSize;
        double u = uPoly_.eval(t);
        double v = vPoly_.eval(t);
        vert.position_ = startVert.position_ + u * forward + v * side;

        double uTangent = uPoly_.evalDerivative(t);
        double vTangent = vPoly_.evalDerivative(t);
        double headingDiff = std::atan2(vTangent, uTangent);

        vert.heading_ = startVert.heading_ + headingDiff;

        tessellation.push_back(vert);
    }
}

ReferenceLine::Vertex ReferenceLine::ParamPoly3::endVertex() const
{
    const Vertex& startVert = startVertex();

    Eigen::Vector2d forward(std::cos(startVert.heading_), std::sin(startVert.heading_));
    Eigen::Vector2d side(-forward.y(), forward.x());

    double endT;
    double endS;
    switch (pRange_)
    {
        default:
            assert(!"invalid p-range");

        case PRange::ARC_LENGTH:
            endT = endS = length();
            break;

        case PRange::NORMALIZED:
            endT = 1;
            endS = length();
            break;
    }

    double endU = uPoly_.eval(endT);
    double endV = vPoly_.eval(endT);

    double endUTangent = uPoly_.evalDerivative(endT);
    double endVTangent = vPoly_.evalDerivative(endT);

    double headingDiff = std::atan2(endVTangent, endUTangent);

    Vertex ret;
    ret.sCoord_ = startVert.sCoord_ + endS;
    ret.position_ = startVert.position_ + endU * forward + endV * side;
    ret.heading_ = startVert.heading_ + headingDiff;
    return ret;
}

}}  // namespace aid::xodr
