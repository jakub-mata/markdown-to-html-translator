#ifndef _STATE_HPP
#define _STATE_HPP

/**
 * @brief To avoid code duplication, e.g. handling <h1>, <h2>,... we can just add to the enum.
 */
struct EnumArithmetic
{
    template <typename enum_t>
    enum_t operator() (enum_t enum_val, int offset) const
    {
        using UnderlyingType = std::underlying_type_t<enum_t>;
        return static_cast<enum_t>(static_cast<UnderlyingType>(enum_val) + offset);
    }
};

/**
 * @brief The enum for state. If you were to create a new state, add a new value to this enum
 * AND creating a corresponding handler in handlers (state_handlers.hpp) and link them
 * together in *state_handlers* vector.
 */
enum State
{
    Data = 0,  // the default state
    DataHashtag, 
    DataAsterisk, 
    DataAsteriskData, 
    DataDoubleAsterisk, 
    DataDoubleAsteriskData, 
    DataTripleAsterisk, 
    DataTripleAsteriskData, 
    DataConsumingNumber, 
    DataOrdinalNumber, 
    HorizontalLine, 
    DataBacktick, 
    DataDoubleBacktick, 
    CodeInline, 
    CodeBlock, 
    UnorderedListPrep, 
    UnorderedList, 
    OrderedListPrep, 
    Image, 
    AltOpenSquared, 
    AltClosedSquared, 
    UrlOpenRound, 
    TitleOpenRound, 
    TitleConsuming, 
    TitleClosedRound, 
    TableHeaderNames, 
    TableHeaderSeparationPipeAwaiting, 
    TableHeaderSeparation, 
    TableCellPipeAwaiting, 
    TableCellData,
};

#endif