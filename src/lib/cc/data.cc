// Copyright (C) 2010-2017 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <cc/data.h>

#include <cstring>
#include <cassert>
#include <climits>
#include <list>
#include <map>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <cerrno>

#include <boost/lexical_cast.hpp>

#include <cmath>

using namespace std;

namespace {
const char* const WHITESPACE = " \b\f\n\r\t";
} // end anonymous namespace

namespace isc {
namespace data {

std::string
Element::Position::str() const {
    std::ostringstream ss;
    ss << file_ << ":" << line_ << ":" << pos_;
    return (ss.str());
}

std::ostream&
operator<<(std::ostream& out, const Element::Position& pos) {
    out << pos.str();
    return (out);
}

std::string
Element::str() const {
    std::stringstream ss;
    toJSON(ss);
    return (ss.str());
}

std::string
Element::toWire() const {
    std::stringstream ss;
    toJSON(ss);
    return (ss.str());
}

void
Element::toWire(std::ostream& ss) const {
    toJSON(ss);
}

bool
Element::getValue(int64_t&) const {
    return (false);
}

bool
Element::getValue(double&) const {
    return (false);
}

bool
Element::getValue(bool&) const {
    return (false);
}

bool
Element::getValue(std::string&) const {
    return (false);
}

bool
Element::getValue(std::vector<ElementPtr>&) const {
    return (false);
}

bool
Element::getValue(std::map<std::string, ConstElementPtr>&) const {
    return (false);
}

bool
Element::setValue(const long long int) {
    return (false);
}

bool
Element::setValue(const double) {
    return (false);
}

bool
Element::setValue(const bool) {
    return (false);
}

bool
Element::setValue(const std::string&) {
    return (false);
}

bool
Element::setValue(const std::vector<ElementPtr>&) {
    return (false);
}

bool
Element::setValue(const std::map<std::string, ConstElementPtr>&) {
    return (false);
}

ConstElementPtr
Element::get(const int) const {
    throwTypeError("get(int) called on a non-list Element");
}

ElementPtr
Element::getNonConst(const int) const {
    throwTypeError("get(int) called on a non-list Element");
}

void
Element::set(const size_t, ElementPtr) {
    throwTypeError("set(int, element) called on a non-list Element");
}

void
Element::add(ElementPtr) {
    throwTypeError("add() called on a non-list Element");
}

void
Element::remove(const int) {
    throwTypeError("remove(int) called on a non-list Element");
}

size_t
Element::size() const {
    throwTypeError("size() called on a non-list Element");
}

bool
Element::empty() const {
    throwTypeError("empty() called on a non-list Element");
}

ConstElementPtr
Element::get(const std::string&) const {
    throwTypeError("get(string) called on a non-map Element");
}

void
Element::set(const std::string&, ConstElementPtr) {
    throwTypeError("set(name, element) called on a non-map Element");
}

void
Element::remove(const std::string&) {
    throwTypeError("remove(string) called on a non-map Element");
}

bool
Element::contains(const std::string&) const {
    throwTypeError("contains(string) called on a non-map Element");
}

ConstElementPtr
Element::find(const std::string&) const {
    throwTypeError("find(string) called on a non-map Element");
}

bool
Element::find(const std::string&, ConstElementPtr&) const {
    return (false);
}

namespace {
inline void
throwJSONError(const std::string& error, const std::string& file, int line,
               int pos)
{
    std::stringstream ss;
    ss << error << " in " + file + ":" << line << ":" << pos;
    isc_throw(JSONError, ss.str());
}
}

std::ostream&
operator<<(std::ostream& out, const Element& e) {
    return (out << e.str());
}

bool
operator==(const Element& a, const Element& b) {
    return (a.equals(b));
}

bool operator!=(const Element& a, const Element& b) {
    return (!a.equals(b));
};

//
// factory functions
//
ElementPtr
Element::create(const Position& pos) {
    return (ElementPtr(new NullElement(pos)));
}

ElementPtr
Element::create(const long long int i, const Position& pos) {
    return (ElementPtr(new IntElement(static_cast<int64_t>(i), pos)));
}

ElementPtr
Element::create(const int i, const Position& pos) {
    return (create(static_cast<long long int>(i), pos));
};

ElementPtr
Element::create(const long int i, const Position& pos) {
    return (create(static_cast<long long int>(i), pos));
};

ElementPtr
Element::create(const double d, const Position& pos) {
    return (ElementPtr(new DoubleElement(d, pos)));
}

ElementPtr
Element::create(const bool b, const Position& pos) {
    return (ElementPtr(new BoolElement(b, pos)));
}

ElementPtr
Element::create(const std::string& s, const Position& pos) {
    return (ElementPtr(new StringElement(s, pos)));
}

ElementPtr
Element::create(const char *s, const Position& pos) {
    return (create(std::string(s), pos));
}

ElementPtr
Element::createList(const Position& pos) {
    return (ElementPtr(new ListElement(pos)));
}

ElementPtr
Element::createMap(const Position& pos) {
    return (ElementPtr(new MapElement(pos)));
}


//
// helper functions for fromJSON factory
//
namespace {
bool
charIn(const int c, const char* chars) {
    const size_t chars_len = std::strlen(chars);
    for (size_t i = 0; i < chars_len; ++i) {
        if (chars[i] == c) {
            return (true);
        }
    }
    return (false);
}

void
skipChars(std::istream& in, const char* chars, int& line, int& pos) {
    int c = in.peek();
    while (charIn(c, chars) && c != EOF) {
        if (c == '\n') {
            ++line;
            pos = 1;
        } else {
            ++pos;
        }
        in.ignore();
        c = in.peek();
    }
}

// skip on the input stream to one of the characters in chars
// if another character is found this function throws JSONError
// unless that character is specified in the optional may_skip
//
// It returns the found character (as an int value).
int
skipTo(std::istream& in, const std::string& file, int& line,
       int& pos, const char* chars, const char* may_skip="")
{
    int c = in.get();
    ++pos;
    while (c != EOF) {
        if (c == '\n') {
            pos = 1;
            ++line;
        }
        if (charIn(c, may_skip)) {
            c = in.get();
            ++pos;
        } else if (charIn(c, chars)) {
            while (charIn(in.peek(), may_skip)) {
                if (in.peek() == '\n') {
                    pos = 1;
                    ++line;
                } else {
                    ++pos;
                }
                in.ignore();
            }
            return (c);
        } else {
            throwJSONError(std::string("'") + std::string(1, c) + "' read, one of \"" + chars + "\" expected", file, line, pos);
        }
    }
    throwJSONError(std::string("EOF read, one of \"") + chars + "\" expected", file, line, pos);
    return (c); // shouldn't reach here, but some compilers require it
}

// TODO: Should we check for all other official escapes here (and
// error on the rest)?
std::string
strFromStringstream(std::istream& in, const std::string& file,
                    const int line, int& pos) throw (JSONError)
{
    std::stringstream ss;
    int c = in.get();
    ++pos;
    if (c == '"') {
        c = in.get();
        ++pos;
    } else {
        throwJSONError("String expected", file, line, pos);
    }

    while (c != EOF && c != '"') {
        if (c == '\\') {
            // see the spec for allowed escape characters
            switch (in.peek()) {
            case '"':
                c = '"';
                break;
            case '/':
                c = '/';
                break;
            case '\\':
                c = '\\';
                break;
            case 'b':
                c = '\b';
                break;
            case 'f':
                c = '\f';
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            default:
                throwJSONError("Bad escape", file, line, pos);
            }
            // drop the escaped char
            in.ignore();
            ++pos;
        }
        ss.put(c);
        c = in.get();
        ++pos;
    }
    if (c == EOF) {
        throwJSONError("Unterminated string", file, line, pos);
    }
    return (ss.str());
}

std::string
wordFromStringstream(std::istream& in, int& pos) {
    std::stringstream ss;
    while (isalpha(in.peek())) {
        ss << (char) in.get();
    }
    pos += ss.str().size();
    return (ss.str());
}

std::string
numberFromStringstream(std::istream& in, int& pos) {
    std::stringstream ss;
    while (isdigit(in.peek()) || in.peek() == '+' || in.peek() == '-' ||
           in.peek() == '.' || in.peek() == 'e' || in.peek() == 'E') {
        ss << (char) in.get();
    }
    pos += ss.str().size();
    return (ss.str());
}

// Should we change from IntElement and DoubleElement to NumberElement
// that can also hold an e value? (and have specific getters if the
// value is larger than an int can handle)
//
ElementPtr
fromStringstreamNumber(std::istream& in, const std::string& file,
                       const int& line, int& pos) {
    // Remember position where the value starts. It will be set in the
    // Position structure of the Element to be created.
    const uint32_t start_pos = pos;
    // This will move the pos to the end of the value.
    const std::string number = numberFromStringstream(in, pos);

    if (number.find_first_of(".eE") < number.size()) {
        try {
            return (Element::create(boost::lexical_cast<double>(number),
                                    Element::Position(file, line, start_pos)));
        } catch (const boost::bad_lexical_cast&) {
            throwJSONError(std::string("Number overflow: ") + number,
                           file, line, start_pos);
        }
    } else {
        try {
            return (Element::create(boost::lexical_cast<int64_t>(number),
                                    Element::Position(file, line, start_pos)));
        } catch (const boost::bad_lexical_cast&) {
            throwJSONError(std::string("Number overflow: ") + number, file,
                           line, start_pos);
        }
    }
    return (ElementPtr());
}

ElementPtr
fromStringstreamBool(std::istream& in, const std::string& file,
                     const int line, int& pos)
{
    // Remember position where the value starts. It will be set in the
    // Position structure of the Element to be created.
    const uint32_t start_pos = pos;
    // This will move the pos to the end of the value.
    const std::string word = wordFromStringstream(in, pos);

    if (word == "true") {
        return (Element::create(true, Element::Position(file, line,
                                                        start_pos)));
    } else if (word == "false") {
        return (Element::create(false, Element::Position(file, line,
                                                         start_pos)));
    } else {
        throwJSONError(std::string("Bad boolean value: ") + word, file,
                       line, start_pos);
    }
    return (ElementPtr());
}

ElementPtr
fromStringstreamNull(std::istream& in, const std::string& file,
                     const int line, int& pos)
{
    // Remember position where the value starts. It will be set in the
    // Position structure of the Element to be created.
    const uint32_t start_pos = pos;
    // This will move the pos to the end of the value.
    const std::string word = wordFromStringstream(in, pos);
    if (word == "null") {
        return (Element::create(Element::Position(file, line, start_pos)));
    } else {
        throwJSONError(std::string("Bad null value: ") + word, file,
                       line, start_pos);
        return (ElementPtr());
    }
}

ElementPtr
fromStringstreamString(std::istream& in, const std::string& file, int& line,
                       int& pos)
{
    // Remember position where the value starts. It will be set in the
    // Position structure of the Element to be created.
    const uint32_t start_pos = pos;
    // This will move the pos to the end of the value.
    const std::string string_value = strFromStringstream(in, file, line, pos);
    return (Element::create(string_value, Element::Position(file, line,
                                                            start_pos)));
}

ElementPtr
fromStringstreamList(std::istream& in, const std::string& file, int& line,
                     int& pos)
{
    int c = 0;
    ElementPtr list = Element::createList(Element::Position(file, line, pos));
    ElementPtr cur_list_element;

    skipChars(in, WHITESPACE, line, pos);
    while (c != EOF && c != ']') {
        if (in.peek() != ']') {
            cur_list_element = Element::fromJSON(in, file, line, pos);
            list->add(cur_list_element);
            c = skipTo(in, file, line, pos, ",]", WHITESPACE);
        } else {
            c = in.get();
            ++pos;
        }
    }
    return (list);
}

ElementPtr
fromStringstreamMap(std::istream& in, const std::string& file, int& line,
                    int& pos)
{
    ElementPtr map = Element::createMap(Element::Position(file, line, pos));
    skipChars(in, WHITESPACE, line, pos);
    int c = in.peek();
    if (c == EOF) {
        throwJSONError(std::string("Unterminated map, <string> or } expected"), file, line, pos);
    } else if (c == '}') {
        // empty map, skip closing curly
        in.ignore();
    } else {
        while (c != EOF && c != '}') {
            std::string key = strFromStringstream(in, file, line, pos);

            skipTo(in, file, line, pos, ":", WHITESPACE);
            // skip the :

            ConstElementPtr value = Element::fromJSON(in, file, line, pos);
            map->set(key, value);

            c = skipTo(in, file, line, pos, ",}", WHITESPACE);
        }
    }
    return (map);
}
} // unnamed namespace

std::string
Element::typeToName(Element::types type) {
    switch (type) {
    case Element::integer:
        return (std::string("integer"));
    case Element::real:
        return (std::string("real"));
    case Element::boolean:
        return (std::string("boolean"));
    case Element::string:
        return (std::string("string"));
    case Element::list:
        return (std::string("list"));
    case Element::map:
        return (std::string("map"));
    case Element::null:
        return (std::string("null"));
    case Element::any:
        return (std::string("any"));
    default:
        return (std::string("unknown"));
    }
}

Element::types
Element::nameToType(const std::string& type_name) {
    if (type_name == "integer") {
        return (Element::integer);
    } else if (type_name == "real") {
        return (Element::real);
    } else if (type_name == "boolean") {
        return (Element::boolean);
    } else if (type_name == "string") {
        return (Element::string);
    } else if (type_name == "list") {
        return (Element::list);
    } else if (type_name == "map") {
        return (Element::map);
    } else if (type_name == "named_set") {
        return (Element::map);
    } else if (type_name == "null") {
        return (Element::null);
    } else if (type_name == "any") {
        return (Element::any);
    } else {
        isc_throw(TypeError, type_name + " is not a valid type name");
    }
}

ElementPtr
Element::fromJSON(std::istream& in, bool preproc) throw(JSONError) {

    int line = 1, pos = 1;
    stringstream filtered;
    if (preproc) {
        preprocess(in, filtered);
    }

    ElementPtr value = fromJSON(preproc ? filtered : in, "<istream>", line, pos);

    return (value);
}

ElementPtr
Element::fromJSON(std::istream& in, const std::string& file_name, bool preproc)
    throw(JSONError)
{
    int line = 1, pos = 1;
    stringstream filtered;
    if (preproc) {
        preprocess(in, filtered);
    }
    return (fromJSON(preproc ? filtered : in, file_name, line, pos));
}

ElementPtr
Element::fromJSON(std::istream& in, const std::string& file, int& line,
                  int& pos) throw(JSONError)
{
    int c = 0;
    ElementPtr element;
    bool el_read = false;
    skipChars(in, WHITESPACE, line, pos);
    while (c != EOF && !el_read) {
        c = in.get();
        pos++;
        switch(c) {
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '0':
            case '-':
            case '+':
            case '.':
                in.putback(c);
                --pos;
                element = fromStringstreamNumber(in, file, line, pos);
                el_read = true;
                break;
            case 't':
            case 'f':
                in.putback(c);
                --pos;
                element = fromStringstreamBool(in, file, line, pos);
                el_read = true;
                break;
            case 'n':
                in.putback(c);
                --pos;
                element = fromStringstreamNull(in, file, line, pos);
                el_read = true;
                break;
            case '"':
                in.putback('"');
                --pos;
                element = fromStringstreamString(in, file, line, pos);
                el_read = true;
                break;
            case '[':
                element = fromStringstreamList(in, file, line, pos);
                el_read = true;
                break;
            case '{':
                element = fromStringstreamMap(in, file, line, pos);
                el_read = true;
                break;
            case EOF:
                break;
            default:
                throwJSONError(std::string("error: unexpected character ") + std::string(1, c), file, line, pos);
                break;
        }
    }
    if (el_read) {
        return (element);
    } else {
        isc_throw(JSONError, "nothing read");
    }
}

ElementPtr
Element::fromJSON(const std::string& in, bool preproc) {
    std::stringstream ss;
    ss << in;

    int line = 1, pos = 1;
    stringstream filtered;
    if (preproc) {
        preprocess(ss, filtered);
    }
    ElementPtr result(fromJSON(preproc ? filtered : ss, "<string>", line, pos));
    skipChars(ss, WHITESPACE, line, pos);
    // ss must now be at end
    if (ss.peek() != EOF) {
        throwJSONError("Extra data", "<string>", line, pos);
    }
    return result;
}

ElementPtr
Element::fromJSONFile(const std::string& file_name,
                      bool preproc) {
    // zero out the errno to be safe
    errno = 0;

    std::ifstream infile(file_name.c_str(), std::ios::in | std::ios::binary);
    if (!infile.is_open())
    {
        const char* error = strerror(errno);
        isc_throw(InvalidOperation, "failed to read file '" << file_name
                  << "': " << error);
    }

    return (fromJSON(infile, file_name, preproc));
}

// to JSON format

void
IntElement::toJSON(std::ostream& ss) const {
    ss << intValue();
}

void
DoubleElement::toJSON(std::ostream& ss) const {
    ss << doubleValue();
}

void
BoolElement::toJSON(std::ostream& ss) const {
    if (boolValue()) {
        ss << "true";
    } else {
        ss << "false";
    }
}

void
NullElement::toJSON(std::ostream& ss) const {
    ss << "null";
}

void
StringElement::toJSON(std::ostream& ss) const {
    ss << "\"";
    const std::string& str = stringValue();
    for (size_t i = 0; i < str.size(); ++i) {
        const char c = str[i];
        // Escape characters as defined in JSON spec
        // Note that we do not escape forward slash; this
        // is allowed, but not mandatory.
        switch (c) {
        case '"':
            ss << '\\' << c;
            break;
        case '\\':
            ss << '\\' << c;
            break;
        case '\b':
            ss << '\\' << 'b';
            break;
        case '\f':
            ss << '\\' << 'f';
            break;
        case '\n':
            ss << '\\' << 'n';
            break;
        case '\r':
            ss << '\\' << 'r';
            break;
        case '\t':
            ss << '\\' << 't';
            break;
        default:
            if ((c >= 0) && (c < 0x20)) {
                std::ostringstream esc;
                esc << "\\u"
                    << hex
                    << setw(4)
                    << setfill('0')
                    << (static_cast<unsigned>(c) & 0xff);
                ss << esc.str();
            } else {
                ss << c;
            }
        }
    }
    ss << "\"";
}

void
ListElement::toJSON(std::ostream& ss) const {
    ss << "[ ";

    const std::vector<ElementPtr>& v = listValue();
    for (std::vector<ElementPtr>::const_iterator it = v.begin();
         it != v.end(); ++it) {
        if (it != v.begin()) {
            ss << ", ";
        }
        (*it)->toJSON(ss);
    }
    ss << " ]";
}

void
MapElement::toJSON(std::ostream& ss) const {
    ss << "{ ";

    const std::map<std::string, ConstElementPtr>& m = mapValue();
    for (std::map<std::string, ConstElementPtr>::const_iterator it = m.begin();
         it != m.end(); ++it) {
        if (it != m.begin()) {
            ss << ", ";
        }
        ss << "\"" << (*it).first << "\": ";
        if ((*it).second) {
            (*it).second->toJSON(ss);
        } else {
            ss << "None";
        }
    }
    ss << " }";
}

// throws when one of the types in the path (except the one
// we're looking for) is not a MapElement
// returns 0 if it could simply not be found
// should that also be an exception?
ConstElementPtr
MapElement::find(const std::string& id) const {
    const size_t sep = id.find('/');
    if (sep == std::string::npos) {
        return (get(id));
    } else {
        ConstElementPtr ce = get(id.substr(0, sep));
        if (ce) {
            // ignore trailing slash
            if  (sep + 1 != id.size()) {
                return (ce->find(id.substr(sep + 1)));
            } else {
                return (ce);
            }
        } else {
            return (ElementPtr());
        }
    }
}

ElementPtr
Element::fromWire(const std::string& s) {
    std::stringstream ss;
    ss << s;
    int line = 0, pos = 0;
    return (fromJSON(ss, "<wire>", line, pos));
}

ElementPtr
Element::fromWire(std::stringstream& in, int) {
    //
    // Check protocol version
    //
    //for (int i = 0 ; i < 4 ; ++i) {
    //    const unsigned char version_byte = get_byte(in);
    //    if (PROTOCOL_VERSION[i] != version_byte) {
    //        throw DecodeError("Protocol version incorrect");
    //    }
    //}
    //length -= 4;
    int line = 0, pos = 0;
    return (fromJSON(in, "<wire>", line, pos));
}

void
MapElement::set(const std::string& key, ConstElementPtr value) {
    m[key] = value;
}

bool
MapElement::find(const std::string& id, ConstElementPtr& t) const {
    try {
        ConstElementPtr p = find(id);
        if (p) {
            t = p;
            return (true);
        }
    } catch (const TypeError&) {
        // ignore
    }
    return (false);
}

bool
IntElement::equals(const Element& other) const {
    return (other.getType() == Element::integer) &&
           (i == other.intValue());
}

bool
DoubleElement::equals(const Element& other) const {
    return (other.getType() == Element::real) &&
           (d == other.doubleValue());
}

bool
BoolElement::equals(const Element& other) const {
    return (other.getType() == Element::boolean) &&
           (b == other.boolValue());
}

bool
NullElement::equals(const Element& other) const {
    return (other.getType() == Element::null);
}

bool
StringElement::equals(const Element& other) const {
    return (other.getType() == Element::string) &&
           (s == other.stringValue());
}

bool
ListElement::equals(const Element& other) const {
    if (other.getType() == Element::list) {
        const size_t s = size();
        if (s != other.size()) {
            return (false);
        }
        for (size_t i = 0; i < s; ++i) {
            if (!get(i)->equals(*other.get(i))) {
                return (false);
            }
        }
        return (true);
    } else {
        return (false);
    }
}

bool
MapElement::equals(const Element& other) const {
    if (other.getType() == Element::map) {
        const std::map<std::string, ConstElementPtr>& m = mapValue();
        for (std::map<std::string, ConstElementPtr>::const_iterator it =
                 m.begin();
             it != m.end() ; ++it) {
            if (other.contains((*it).first)) {
                if (!get((*it).first)->equals(*other.get((*it).first))) {
                    return (false);
                }
            } else {
                return (false);
            }
        }
        // quickly walk through the other map too, to see if there's
        // anything in there that we don't have. We don't need to
        // compare those elements; if one of them is missing we
        // differ (and if it's not missing the loop above has checked
        // it)
        std::map<std::string, ConstElementPtr>::const_iterator it;
        for (it = other.mapValue().begin();
             it != other.mapValue().end();
             ++it) {
            if (!contains((*it).first)) {
                return (false);
            }
        }
        return (true);
    } else {
        return (false);
    }
}

bool
isNull(ConstElementPtr p) {
    return (!p);
}

void
removeIdentical(ElementPtr a, ConstElementPtr b) {
    if (!b) {
        return;
    }
    if (a->getType() != Element::map || b->getType() != Element::map) {
        isc_throw(TypeError, "Non-map Elements passed to removeIdentical");
    }

    // As maps do not allow entries with multiple keys, we can either iterate
    // over a checking for identical entries in b or vice-versa.  As elements
    // are removed from a if a match is found, we choose to iterate over b to
    // avoid problems with element removal affecting the iterator.
    const std::map<std::string, ConstElementPtr>& m = b->mapValue();
    for (std::map<std::string, ConstElementPtr>::const_iterator it = m.begin();
         it != m.end() ; ++it) {
        if (a->contains((*it).first)) {
            if (a->get((*it).first)->equals(*b->get((*it).first))) {
                a->remove((*it).first);
            }
        }
    }
}

ConstElementPtr
removeIdentical(ConstElementPtr a, ConstElementPtr b) {
    ElementPtr result = Element::createMap();

    if (!b) {
        return (result);
    }

    if (a->getType() != Element::map || b->getType() != Element::map) {
        isc_throw(TypeError, "Non-map Elements passed to removeIdentical");
    }

    const std::map<std::string, ConstElementPtr>& m = a->mapValue();
    for (std::map<std::string, ConstElementPtr>::const_iterator it = m.begin();
         it != m.end() ; ++it) {
        if (!b->contains((*it).first) ||
            !a->get((*it).first)->equals(*b->get((*it).first))) {
            result->set((*it).first, (*it).second);
        }
    }

    return (result);
}

void
merge(ElementPtr element, ConstElementPtr other) {
    if (element->getType() != Element::map ||
        other->getType() != Element::map) {
        isc_throw(TypeError, "merge arguments not MapElements");
    }

    const std::map<std::string, ConstElementPtr>& m = other->mapValue();
    for (std::map<std::string, ConstElementPtr>::const_iterator it = m.begin();
         it != m.end() ; ++it) {
        if ((*it).second && (*it).second->getType() != Element::null) {
            element->set((*it).first, (*it).second);
        } else if (element->contains((*it).first)) {
            element->remove((*it).first);
        }
    }
}

ElementPtr
copy(ConstElementPtr from, int level) {
    if (isNull(from)) {
        isc_throw(BadValue, "copy got a null pointer");
    }
    int from_type = from->getType();
    if (from_type == Element::integer) {
        return (ElementPtr(new IntElement(from->intValue())));
    } else if (from_type == Element::real) {
        return (ElementPtr(new DoubleElement(from->doubleValue())));
    } else if (from_type == Element::boolean) {
        return (ElementPtr(new BoolElement(from->boolValue())));
    } else if (from_type == Element::null) {
        return (ElementPtr(new NullElement()));
    } else if (from_type == Element::string) {
        return (ElementPtr(new StringElement(from->stringValue())));
    } else if (from_type == Element::list) {
        ElementPtr result = ElementPtr(new ListElement());
        typedef std::vector<ElementPtr> ListType;
        const ListType& value = from->listValue();
        for (ListType::const_iterator it = value.cbegin();
             it != value.cend(); ++it) {
            if (level == 0) {
                result->add(*it);
            } else {
                result->add(copy(*it, level - 1));
            }
        }
        return (result);
    } else if (from_type == Element::map) {
        ElementPtr result = ElementPtr(new MapElement());
        typedef std::map<std::string, ConstElementPtr> MapType;
        const MapType& value = from->mapValue();
        for (MapType::const_iterator it = value.cbegin();
             it != value.cend(); ++it) {
            if (level == 0) {
                result->set(it->first, it->second);
            } else {
                result->set(it->first, copy(it->second, level - 1));
            }
        }
        return (result);
    } else {
        isc_throw(BadValue, "copy got an element of type: " << from_type);
    }
}

namespace {

// Helper function which blocks infinite recursion
bool
isEquivalent0(ConstElementPtr a, ConstElementPtr b, unsigned level)
{
    // check looping forever on cycles
    if (!level) {
        isc_throw(BadValue, "isEquivalent got infinite recursion: "
                  "arguments include cycles");
    }
    if (!a || !b) {
        isc_throw(BadValue, "isEquivalent got a null pointer");
    }
    // check types
    if (a->getType() != b->getType()) {
        return (false);
    }
    if (a->getType() == Element::list) {
        // check empty
        if (a->empty()) {
            return (b->empty());
        }
        // check size
        if (a->size() != b->size()) {
            return (false);
        }

        // copy b into a list
        const size_t s = a->size();
        typedef std::list<ConstElementPtr> ListType;
        ListType l;
        for (size_t i = 0; i < s; ++i) {
            l.push_back(b->get(i));
        }

        // iterate on a
        for (size_t i = 0; i < s; ++i) {
            ConstElementPtr item = a->get(i);
            // lookup this item in the list
            bool found = false;
            for (ListType::iterator it = l.begin();
                 it != l.end(); ++it) {
                // if found in the list remove it
                if (isEquivalent0(item, *it, level - 1)) {
                    found = true;
                    l.erase(it);
                    break;
                }
            }
            // if not found argument differs
            if (!found) {
                return (false);
            }
        }

        // sanity check: the list must be empty
        if (!l.empty()) {
            isc_throw(Unexpected, "isEquivalent internal error");
        }
        return (true);
    } else if (a->getType() == Element::map) {
        // iterate on the first map
        typedef std::map<std::string, ConstElementPtr> MapType;
        const MapType& ma = a->mapValue();
        for (MapType::const_iterator it = ma.begin();
             it != ma.end() ; ++it) {
            // get the b value for the given keyword and recurse
            ConstElementPtr item = b->get(it->first);
            if (!item || !isEquivalent0(it->second, item, level - 1)) {
                return (false);
            }
        }
        // iterate on the second map
        const MapType& mb = b->mapValue();
        for (MapType::const_iterator it = mb.begin();
             it != mb.end() ; ++it) {
            // check if the keyword exists
            if (!a->contains(it->first)) {
                return (false);
            }
        }
        return (true);
    } else {
        return (a->equals(*b));
    }
}

}

bool
isEquivalent(ConstElementPtr a, ConstElementPtr b) {
    return (isEquivalent0(a, b, 100));
}

void
prettyPrint(ConstElementPtr element, std::ostream& out,
            unsigned indent, unsigned step) {
    if (!element) {
        isc_throw(BadValue, "prettyPrint got a null pointer");
    }
    if (element->getType() == Element::list) {
        // empty list case
        if (element->empty()) {
            out << "[ ]";
            return;
        }

        // complex ? multiline : oneline
        if (!element->get(0)) {
            isc_throw(BadValue, "prettyPrint got a null pointer");
        }
        int first_type = element->get(0)->getType();
        bool complex = false;
        if ((first_type == Element::list) || (first_type == Element::map)) {
            complex = true;
        }
        std::string separator = complex ? ",\n" : ", ";

        // open the list
        out << "[" << (complex ? "\n" : " ");
        
        // iterate on items
        typedef std::vector<ElementPtr> ListType;
        const ListType& l = element->listValue();
        for (ListType::const_iterator it = l.begin();
             it != l.end(); ++it) {
            // add the separator if not the first item
            if (it != l.begin()) {
                out << separator;
            }
            // add indentation
            if (complex) {
                out << std::string(indent + step, ' ');
            }
            // recursive call
            prettyPrint(*it, out, indent + step, step);
        }

        // close the list
        if (complex) {
            out << "\n" << std::string(indent, ' ');
        } else {
            out << " ";
        }
        out << "]";
    } else if (element->getType() == Element::map) {
        // empty map case
        if (element->size() == 0) {
            out << "{ }";
            return;
        }

        // open the map
        out << "{\n";

        // iterate on keyword: value
        typedef std::map<std::string, ConstElementPtr> MapType;
        const MapType& m = element->mapValue();
        for (MapType::const_iterator it = m.begin();
             it != m.end(); ++it) {
            // add the separator if not the first item
            if (it != m.begin()) {
                out << ",\n";
            }
            // add indentation
            out << std::string(indent + step, ' ');
            // add keyword:
            out << "\"" << it->first << "\": ";
            // recusive call
            prettyPrint(it->second, out, indent + step, step);
        }

        // close the map
        out << "\n" << std::string(indent, ' ') << "}";
    } else {
        // not a list or a map
        element->toJSON(out);
    }
}

std::string
prettyPrint(ConstElementPtr element, unsigned indent, unsigned step) {
    std::stringstream ss;
    prettyPrint(element, ss, indent, step);
    return (ss.str());
}

void Element::preprocess(std::istream& in, std::stringstream& out) {

    std::string line;

    while (std::getline(in, line)) {
        // If this is a comments line, replace it with empty line
        // (so the line numbers will still match
        if (!line.empty() && line[0] == '#') {
            line = "";
        }

        // getline() removes end line characters. Unfortunately, we need
        // it for getting the line numbers right (in case we report an
        // error.
        out << line;
        out << "\n";
    }
}

}
}
