#include <eui/euiTextSearcher.h>
namespace eui {
TextSearcher::~TextSearcher() = default;
// pMessages supplies localized strings; pProcessor handles embedded formatting tags.
TextSearcher::TextSearcher(const MessageSet* pMessages, TagProcessor* pProcessor)
    : mMessages(pMessages), mProcessor(pProcessor) {}
}
