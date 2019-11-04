#include "lane_section.h"

#include <cmath>
#include <climits>

namespace aid { namespace xodr {

LaneSection::Lane::Lane() : predecessor_(LaneIDOpt::null()), successor_(LaneIDOpt::null()) {}

std::vector<LaneSection::BoundaryTessellation> LaneSection::tessellateLaneBoundaries(
    const ReferenceLine::Tessellation& refLineTessellation) const
{
    assert(!refLineTessellation.empty());
    assert(numLeftLanes_ <= static_cast<int>(lanes_.size()));

    std::vector<BoundaryTessellation> ret;
    ret.resize(lanes_.size() + 1);

    std::vector<double>& centerOut = ret[numLeftLanes_].lateralPositions_;
    centerOut.resize(refLineTessellation.size());
    std::fill(centerOut.begin(), centerOut.end(), 0);

    // Tessellate the left lanes.
    if (numLeftLanes_)
    {
        tessellateLaneBoundariesSide(refLineTessellation, ret.data() + numLeftLanes_, numLeftLanes_ - 1, -1, -1);
    }

    // Tessellate the right lanes.
    if (numLeftLanes_ < static_cast<int>(lanes_.size()))
    {
        tessellateLaneBoundariesSide(refLineTessellation, ret.data() + numLeftLanes_, numLeftLanes_, lanes_.size(), 1);
    }

    return ret;
}

void LaneSection::tessellateLaneBoundariesSide(const ReferenceLine::Tessellation& refLineTessellation,
                                               BoundaryTessellation* boundaryIt, int lanesBegin, int lanesEnd,
                                               int stepDir) const
{
    assert(stepDir == -1 || stepDir == 1);

    // lanesEnd < lanesBegin implies stepDir must be -1.
    assert(!(lanesEnd < lanesBegin) || stepDir == -1);

    // lanesEnd > lanesBegin implies stepDir must be 1.
    assert(!(lanesEnd > lanesBegin) || stepDir == 1);

    BoundaryTessellation* prevBoundaryIt = boundaryIt;
    boundaryIt += stepDir;

    for (int i = lanesBegin; i != lanesEnd; i += stepDir)
    {
        auto refLineIt = refLineTessellation.begin();

        auto innerBoundaryIt = prevBoundaryIt->lateralPositions_.begin();

        boundaryIt->lateralPositions_.resize(refLineTessellation.size());
        auto outerBoundaryIt = boundaryIt->lateralPositions_.begin();

        auto curPolyIt = lanes_[i].widthPoly3s_.begin();
        auto endPolyIt = lanes_[i].widthPoly3s_.end();
        auto nextPolyIt = curPolyIt + 1;

        while (refLineIt != refLineTessellation.end())
        {
            double param = refLineIt->sCoord_ - startS_;

            // If we're past the sOffset_ of the next polynomial (if one exists),
            // then keep advancing until we found the polynomial our current param lies in.
            while (nextPolyIt != endPolyIt && param >= nextPolyIt->sOffset())
            {
                curPolyIt = nextPolyIt;
                nextPolyIt++;
            }

            double ds = param - curPolyIt->sOffset();
            *outerBoundaryIt = *innerBoundaryIt + curPolyIt->poly3().eval(ds) * -stepDir;

            refLineIt++;
            innerBoundaryIt++;
            outerBoundaryIt++;
        }

        prevBoundaryIt = boundaryIt;
        boundaryIt += stepDir;
    }
}

std::vector<LaneSection::BoundaryCurveTessellation> LaneSection::tessellateLaneBoundaryCurves(
    const ReferenceLine::Tessellation& refLineTessellation) const
{
    assert(!refLineTessellation.empty());

    auto boundaries = tessellateLaneBoundaries(refLineTessellation);

    int numPoints = static_cast<int>(refLineTessellation.size());
    int numBoundaries = static_cast<int>(boundaries.size());

    std::vector<BoundaryCurveTessellation> ret;
    ret.resize(numBoundaries);
    for (int i = 0; i < numBoundaries; i++)
    {
        ret[i].vertices_.resize(numPoints);
    }

    for (int i = 0; i < numPoints; i++)
    {
        Eigen::Vector2d pt = refLineTessellation[i].position_;
        double heading = refLineTessellation[i].heading_;

        Eigen::Vector2d perp(-std::sin(heading), std::cos(heading));

        for (int j = 0; j < numBoundaries; j++)
        {
            double lateral = boundaries[j].lateralPositions_[i];
            ret[j].vertices_[i] = pt + perp * lateral;
        }
    }

    return ret;
}

std::vector<LaneSection::CenterLineTessellation> LaneSection::tessellateLaneCenterLines(
    const ReferenceLine::Tessellation& refLineTessellation) const
{
    assert(!refLineTessellation.empty());

    std::vector<LaneSection::CenterLineTessellation> ret;

    auto boundaries = tessellateLaneBoundaries(refLineTessellation);

    int numPoints = refLineTessellation.size();
    int numLanes = boundaries.size() - 1;
    ret.resize(numLanes);
    for (int i = 0; i < numLanes; i++)
    {
        ret[i].vertices_.resize(numPoints);
        ret[i].variances_.resize(numPoints);
    }

    for (int i = 0; i < numPoints; i++)
    {
        Eigen::Vector2d pt = refLineTessellation[i].position_;
        double heading = refLineTessellation[i].heading_;

        Eigen::Vector2d perp(-std::sin(heading), std::cos(heading));

        for (int j = 0; j < numLanes; j++)
        {
            double variance = .5f * (boundaries[j + 1].lateralPositions_[i] - boundaries[j].lateralPositions_[i]);

            double centerLineLateral = boundaries[j].lateralPositions_[i] + variance;
            ret[j].vertices_[i] = pt + perp * centerLineLateral;
            ret[j].variances_[i] = variance;
        }
    }

    return ret;
}

LaneSection::BoundaryCurveAndCenterLineTessellations LaneSection::tessellateLaneBoundaryCurvesAndCenterLines(
    const ReferenceLine::Tessellation& refLineTessellation) const
{
    assert(!refLineTessellation.empty());

    BoundaryCurveAndCenterLineTessellations ret;

    auto boundaries = tessellateLaneBoundaries(refLineTessellation);

    int numPoints = refLineTessellation.size();
    int numBoundaries = static_cast<int>(boundaries.size());
    int numLanes = numBoundaries - 1;

    ret.boundaryCurveTessellations_.resize(numBoundaries);
    for (int i = 0; i < numBoundaries; i++)
    {
        ret.boundaryCurveTessellations_[i].vertices_.resize(numPoints);
    }

    ret.centerLineTessellations_.resize(numLanes);
    for (int i = 0; i < numLanes; i++)
    {
        ret.centerLineTessellations_[i].vertices_.resize(numPoints);
        ret.centerLineTessellations_[i].variances_.resize(numPoints);
    }

    for (int i = 0; i < numPoints; i++)
    {
        Eigen::Vector2d pt = refLineTessellation[i].position_;
        double heading = refLineTessellation[i].heading_;

        Eigen::Vector2d perp(-std::sin(heading), std::cos(heading));

        for (int j = 0; j < numBoundaries; j++)
        {
            double lateral = boundaries[j].lateralPositions_[i];
            ret.boundaryCurveTessellations_[j].vertices_[i] = pt + perp * lateral;
        }

        for (int j = 0; j < numLanes; j++)
        {
            double variance = .5f * (boundaries[j + 1].lateralPositions_[i] - boundaries[j].lateralPositions_[i]);

            double centerLineLateral = boundaries[j].lateralPositions_[i] + variance;
            ret.centerLineTessellations_[j].vertices_[i] = pt + perp * centerLineLateral;
            ret.centerLineTessellations_[j].variances_[i] = variance;
        }
    }

    return ret;
}

LaneID LaneSection::laneIndexToId(int idx) const
{
    assert(idx >= 0 && idx < static_cast<int>(lanes_.size()));

    // In XODR, lane ID's are consecutive and descending (when going from left
    // to right), and the lane with ID 0 should be the center lane.
    //
    // Our lanes_ vector stores the lanes from left to right, and omits the
    // center lane.

    // Change to indices which have the left most lane at index 'numLeftLanes_'
    // and are descending instead of ascending.
    int id = numLeftLanes_ - idx;

    // If the lane is to the right of the center line, then subtract 1, to
    // account for the fact that our lanes_ array doesn't include the center
    // line, while XODR's id range does have a ID reserved for it (ID 0).
    if (id <= 0)
    {
        id--;
    }

    return LaneID(id);
}

int LaneSection::laneIdToIndex(LaneID id) const
{
    int idInt = static_cast<int>(id);

    assert(idInt != 0);
    assert(idInt <= numLeftLanes_);
    assert(idInt >= -numRightLanes());

    // In XODR, lane ID's are consecutive and descending (when going from left
    // to right), and the lane with ID 0 should be the center lane.
    //
    // Our lanes_ vector stores the lanes from left to right, and omits the
    // center lane.

    // Change to indices which have the left most lane at index 0 and are
    // ascending instead of descending.
    int idx = numLeftLanes_ - idInt;

    // If the lane is to the right of the center line, then subtract 1, to
    // account for the fact that our lanes_ array doesn't include the center
    // line, while XODR's id range does have a ID reserved for it (ID 0).
    if (idx >= numLeftLanes_)
    {
        idx--;
    }

    return idx;
}

void LaneSection::validate() const
{
    double maxSOffset = endS_ - startS_;
    for (const Lane& lane : lanes_)
    {
        lane.validate(maxSOffset);
    }
}

bool LaneSection::Lane::hasPredecessor() const
{
    return static_cast<bool>(predecessor_);
}

LaneID LaneSection::Lane::predecessor() const
{
    return *predecessor_;
}

bool LaneSection::Lane::hasSuccessor() const
{
    return static_cast<bool>(successor_);
}

LaneID LaneSection::Lane::successor() const
{
    return *successor_;
}

bool LaneSection::Lane::hasLink(RoadLinkType roadLinkType) const
{
    switch (roadLinkType)
    {
        default:
            assert(!"invalid roadLinkType");

        case RoadLinkType::PREDECESSOR:
            return static_cast<bool>(predecessor_);

        case RoadLinkType::SUCCESSOR:
            return static_cast<bool>(successor_);
    }
}

LaneID LaneSection::Lane::link(RoadLinkType roadLinkType) const
{
    switch (roadLinkType)
    {
        default:
            assert(!"invalid roadLinkType");

        case RoadLinkType::PREDECESSOR:
            return *predecessor_;

        case RoadLinkType::SUCCESSOR:
            return *successor_;
    }
}

template <class T>
static void validateAttribSCoords(const std::string& attribsName, double maxSOffset, const std::vector<T>& attribs)
{
    if (attribs.empty())
    {
        return;
    }

    if (attribs.front().sOffset() < 0 || attribs.back().sOffset() >= maxSOffset)
    {
        std::stringstream err;
        err << "The s-offset of the <" << attribsName << "> elements of a lane should fall within the lane's s-range.";
        throw std::runtime_error(err.str());
    }

    for (size_t i = 0; i < attribs.size() - 1; i++)
    {
        if (attribs[i].sOffset() >= attribs[i + 1].sOffset())
        {
            std::stringstream err;
            err << "The <" << attribsName << "> elements of a lane should occur in increasing s-offset order.";
            throw std::runtime_error(err.str());
        }
    }
}

void LaneSection::Lane::validate(double maxSOffset) const
{
    validateAttribSCoords("width", maxSOffset, widthPoly3s_);
    validateAttribSCoords("material", maxSOffset, materials_);
    validateAttribSCoords("visibility", maxSOffset, visibilities_);
    validateAttribSCoords("speed", maxSOffset, speedLimits_);
    validateAttribSCoords("access", maxSOffset, accesses_);
    validateAttribSCoords("height", maxSOffset, heights_);
    validateAttribSCoords("rule", maxSOffset, rules_);
}

double LaneSection::Lane::widthAtSCoord(const double s) const
{
    size_t polyIdx;
    for (polyIdx = 1; polyIdx < widthPoly3s_.size(); polyIdx++)
    {
        const auto& poly = widthPoly3s_[polyIdx];
        if (s < poly.sOffset())
        {
            break;
        }
    }

    // last poly where sOffset >= poly.sOffset()
    const auto& poly = widthPoly3s_[polyIdx - 1];
    auto param = s - poly.sOffset();
    return poly.poly3().eval(param);
}

}}  // namespace aid::xodr
