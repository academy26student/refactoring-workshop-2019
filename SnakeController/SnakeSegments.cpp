#include "SnakeSegments.hpp"
#include "EventT.hpp"
#include <algorithm>

namespace Snake
{

namespace
{
bool isHorizontal(Direction direction)
{
    return Direction_LEFT == direction or Direction_RIGHT == direction;
}

bool isVertical(Direction direction)
{
    return Direction_UP == direction or Direction_DOWN == direction;
}

bool isPositive(Direction direction)
{
    return (isVertical(direction) and Direction_DOWN == direction)
        or (isHorizontal(direction) and Direction_RIGHT == direction);
}

bool perpendicular(Direction dir1, Direction dir2)
{
    return isHorizontal(dir1) == isVertical(dir2);
}
} // namespace

Segments::Segments(Direction direction)
    : m_headDirection(direction)
{}

void Segments::addSegment(Position position)
{
    m_segments.emplace_back(position);
}

bool Segments::isCollision(Position position) const
{
    return m_segments.end() !=  std::find_if(m_segments.cbegin(), m_segments.cend(),
        [position](auto const& segment){ return segment.x == position.x and segment.y == position.y; });
}

void Segments::addHead(Position position)
{
    m_segments.push_front(position);
}

Position Segments::removeTail()
{
    auto tail = m_segments.back();
    m_segments.pop_back();
    return tail;
}

Position Segments::nextHead() const
{
    Position const& currentHead = m_segments.front();

    Position newHead;
    newHead.x = currentHead.x + (isHorizontal(m_headDirection) ? isPositive(m_headDirection) ? 1 : -1 : 0);
    newHead.y = currentHead.y + (isVertical(m_headDirection) ? isPositive(m_headDirection) ? 1 : -1 : 0);

    return newHead;
}

void Segments::updateDirection(Direction newDirection)
{
    if (perpendicular(m_headDirection, newDirection)) {
        m_headDirection = newDirection;
    }
}

unsigned Segments::size() const
{
    return m_segments.size();
}

void Segments::removeTailSegment(IPort& m_displayPort)
{
    auto tailPosition = removeTail();

    DisplayInd clearTail;
    clearTail.position = tailPosition;
    clearTail.value = Cell_FREE;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(clearTail));
}

void Segments::addHeadSegment(Position position, IPort& m_displayPort)
{
    addHead(position);

    DisplayInd placeNewHead;
    placeNewHead.position = position;
    placeNewHead.value = Cell_SNAKE;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(placeNewHead));
}

void Segments::removeTailSegmentIfNotScored(Position position, World m_world,IPort& m_scorePort,IPort&  m_foodPort, IPort& m_displayPort)
{
    if (position == m_world.getFoodPosition()) {
        ScoreInd scoreIndication{size() - 1};
        m_scorePort.send(std::make_unique<EventT<ScoreInd>>(scoreIndication));
        m_foodPort.send(std::make_unique<EventT<FoodReq>>());
    } else {
        removeTailSegment(m_displayPort);
    }
}

void Segments::updateSegmentsIfSuccessfullMove(Position position,World m_world, IPort& m_scorePort,IPort&  m_foodPort, IPort& m_displayPort)
{
    if (isCollision(position) or not m_world.contains(position)) {
        m_scorePort.send(std::make_unique<EventT<LooseInd>>());
    } else {
        addHeadSegment(position, m_displayPort);
        removeTailSegmentIfNotScored(position, m_world,  m_scorePort,m_foodPort, m_displayPort);
    }
}
} // namespace Snake