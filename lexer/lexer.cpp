#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Value.h"
#include <cctype>
#include <cstdlib>
#include <exception>
#include <iterator>
#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <list>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalIFunc.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/Error.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <locale>
#include <ostream>
#include <queue>
#include <range/v3/algorithm/contains.hpp>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/iterator/common_iterator.hpp>
#include <range/v3/iterator/concepts.hpp>
#include <range/v3/iterator/traits.hpp>
#include <range/v3/range/concepts.hpp>
#include <range/v3/range/traits.hpp>
#include <range/v3/view.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/istream.hpp>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include <variant>
#include <unordered_map>
#include <fstream>
#include <deque>
#include <source_location>
#include <llvm/ADT/Hashing.h>
#include <llvm/Support/FileSystem.h>
#include <string_view>
#ifdef _WIN32
#include <windows.h>
#endif

unsigned constexpr stringhash(char const* input) {
	return *input ? static_cast<unsigned int> (*input) +
		33 * stringhash(input + 1)
		: 5381;
}

constexpr inline auto operator"" _h(char const* p, size_t) {
	return stringhash(p);
}

static std::string::iterator currlexing;

extern std::string subject;

extern "C" void docall(const char* name, size_t szname, void* phashmap);

typedef std::list<std::pair<std::string, std::unordered_map<unsigned, std::string>>> record;

static size_t recording;

struct return_t {
	bool result;
	record record;
	std::unordered_map<unsigned, std::string> matches;

	bool handle(std::unordered_map<unsigned, std::string>& priormatches, ::record& currrecord) {

		currrecord.splice(currrecord.end(), record);

		if (!matches.empty()) {
			priormatches = matches;
		}

		return result;
	}
};

void doit(std::string fnname, void* phashmap, record &refrecord) {

	if (recording) {
		docall(fnname.c_str(), fnname.length(), phashmap);
	}
	else {
		refrecord.push_back({ fnname.c_str(), *static_cast<const std::unordered_map<unsigned, std::string>*>(phashmap) });
	}
}

static void replay(record &what) {

	if (recording);
	else for (const auto& call : what) {
		doit(call.first, (void*)&call.second, what);
	}

	what.clear();
}

bool consumewhitespace() {
	while (isspace(*currlexing)) currlexing++;
	return true;
}

return_t escape(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flags) {
	priormatches.clear();

	record currrecord;

	auto doit = [&](std::string what) {
		::doit(what, (void*)&priormatches, currrecord);
	};

	if (*currlexing == '\\') {
		auto begin = ++currlexing;

		switch (*currlexing) if (0);
		else if (0) {
		case 'x':
			while (isxdigit(*++currlexing));
		}
		else if (0) {
		case '0':
			while (ranges::contains(std::string{ "01234567" }, *++currlexing));
		}
		else if (0) {
		default:
			if (std::string{ "\'\"?\\\\abfnrtv0" }.contains(*currlexing))
				++currlexing;
		}

		priormatches["escaperaw"_h] = { begin, currlexing };

		doit("addescapesequencetostring");

		assert(currlexing != begin);

		return { true, currrecord };
	}

	return { false, currrecord };
}

return_t stringlit(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag) {
	priormatches.clear();
	consumewhitespace();

	record currrecord;

	auto doit = [&](std::string what) {
		::doit(what, (void*)&priormatches, currrecord);
	};

	auto call = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
		return what(priormatches, flag).handle(priormatches, currrecord);
	};

	if (ranges::contains(std::array{ '\'', '\"' }, *currlexing)) {
		priormatches["begincharliteral"_h] = *currlexing++;
		auto lastnonescapebegin = currlexing;

		for (;;) {
			std::array potrawtext = { lastnonescapebegin, currlexing };

			auto checkrawcharcompletion = [&] {
				if (potrawtext[0] != potrawtext[1]) {
					priormatches["textraw"_h] = std::string{ potrawtext[0], potrawtext[1] };
					doit("addplaintexttostring");
				}
			};

			++recording;

			if (call(escape)) {
				lastnonescapebegin = currlexing;
				--recording;
				replay(currrecord);
				checkrawcharcompletion();
			}
			replay(currrecord);
			--recording;
			
			if (priormatches["begincharliteral"_h][0] == *currlexing) {
				checkrawcharcompletion();
				break;
			}
			else {
				currlexing++;
			}
		}
		doit("add_literal");
		consumewhitespace();
		return { true, currrecord };
	}

	return { false, currrecord };
}

return_t numberliteral(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag) {
	priormatches.clear();
	consumewhitespace();

	record currrecord;

	auto doit = [&](std::string what) {
		::doit(what, (void*)&priormatches, currrecord);
	};

	auto call = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag).handle(priormatches, currrecord);
	};

	switch (stringhash(std::string{ currlexing, currlexing + 1 }.c_str())) if (0);
	else if (0) {
	case "0x"_h:
		currlexing += 2;
		auto beg = currlexing;
		while (isxdigit(*currlexing++));
		priormatches["hex"_h] = std::string{ beg , currlexing };
	}
	else if (0) {
	case "0b"_h:
		currlexing += 2;
		auto beg = currlexing;
		while (ranges::contains(std::string{"01"}, *currlexing++));
		priormatches["bin"_h] = std::string{ beg , currlexing };
	}
	else if (0) {
	default:
		if (*currlexing == '0' && isdigit(currlexing[1])) {
			auto beg = currlexing;
			currlexing += 2;
			while (ranges::contains(std::string{ "01234567" }, *++currlexing));
			priormatches["oct"_h] = std::string{ beg , currlexing };
		}
		else if (isdigit(*currlexing)) {
			auto beg = currlexing;
			while (isdigit(*++currlexing));
			priormatches["dec"_h] = std::string{ beg , currlexing };
		}
		else {
			return { false, currrecord };
		}
	}

	{
		auto beg = currlexing;

		if (ranges::contains(std::string{ "lL" }, *currlexing)) {
			++currlexing;
			if (*currlexing == currlexing[-1])
				++currlexing;
			if (ranges::contains(std::string{ "uU" }, *currlexing)) {
				++currlexing;
			}
		}
		else if (ranges::contains(std::string{ "uU" }, *currlexing)) {
			++currlexing;
			if (ranges::contains(std::string{ "lL" }, *currlexing)) {
				++currlexing;
				if (*currlexing == currlexing[-1])
					++currlexing;
			}
		}

		if (beg != currlexing) {
			priormatches["lng"_h] = std::string{ beg , currlexing };
		}

		doit("num_lit");

		consumewhitespace();

		return { true, currrecord };
	}
}

return_t exponent(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag) {


	record currrecord;

	auto doit = [&](std::string what) {
		::doit(what, (void*)&priormatches, currrecord);
	};

	auto call = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag).handle(priormatches, currrecord);
	};

	if (ranges::contains(std::string{ "eE" }, *currlexing)) {
		auto beg = ++currlexing;
		if (ranges::contains(std::string{ "+-" }, *currlexing)) {
			priormatches["signexp"_h] = std::string{ beg, ++currlexing };
		}
		beg = currlexing;
		while (isdigit(*currlexing++));
		priormatches["exponent"_h] = std::string{ beg , currlexing };

		return { true, currrecord, priormatches };
	}

	return { false, currrecord };
}

return_t floating(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag) {
	priormatches.clear();
	consumewhitespace();

	record currrecord;

	auto doit = [&](std::string what) {
		::doit(what, (void*)&priormatches, currrecord);
	};

	auto call = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag).handle(priormatches, currrecord);
	};

	if(*currlexing == '.') 
fraction:
	{
		auto beg = ++currlexing;
		while (isdigit(*currlexing++));
		priormatches["fraction"_h] = std::string{ beg , currlexing };
		call(exponent);
		goto success;
	}
	else if (isdigit(*currlexing)) {
		auto beg = currlexing;
		while (isdigit(*++currlexing));
		priormatches["whole"_h] = std::string{ beg , currlexing };
		if (*currlexing == '.') {
			goto fraction;
		}
		else if(call(exponent)) {
			goto success;
		}
	}
	else if (std::string{ currlexing, currlexing + 3 } == "NAN") {
		priormatches["nan"_h] = std::string{ currlexing, currlexing + 3 };
		currlexing += 3;
		goto success;
	}
	return { false, currrecord };
success:
	{
		auto beg = currlexing;

		if (ranges::contains(std::string{ "flFL" }, *currlexing)) ++currlexing;

		if (beg != currlexing) {
			priormatches["suffixflt"_h] = std::string{ beg , currlexing };
		}

		doit("collect_float_literal");

		consumewhitespace();

		return { true, currrecord };
	}
}

return_t identifier(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag) {
	consumewhitespace();

	if (isalpha(*currlexing) || *currlexing == '_') {
		auto beg = currlexing;
		++currlexing;
		while (isalnum(*currlexing) || *currlexing == '_') ++currlexing;
		priormatches["ident"_h] = std::string{ beg , currlexing };
		consumewhitespace();

		return { true, {}, priormatches };
	}

	return { false };
}

return_t qualifiersidentifier(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag) {
	priormatches.clear();
	consumewhitespace();

	record currrecord;

	auto doit = [&](std::string what) {
		::doit(what, (void*)&priormatches, currrecord);
	};

	auto call = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag).handle(priormatches, currrecord);
	};

	auto last = currlexing;

	if (call(identifier)) {
		switch (stringhash(priormatches["ident"_h].c_str())) if (0);
		else if (0) {
		case "const"_h:
		case "restrict"_h:
		case "volatile"_h:
		case "extern"_h:
		case "auto"_h:
		case "static"_h:
		case "register"_h:
			priormatches["qualiffound"_h] = std::move(priormatches["ident"_h]);
			doit("add_qualif");
			consumewhitespace();

			return { true, currrecord };
		}
	}

	currlexing = last;

	return { false, currrecord };
}

return_t Tinside(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag);
return_t abstrsubs(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag);

return_t abstrptrrev(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag) {
	priormatches.clear();
	consumewhitespace();

	record currrecord;

	auto doit = [&](std::string what) {
		::doit(what, (void*)&priormatches, currrecord);
	};

	auto call = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag).handle(priormatches, currrecord);
	};

	if (*currlexing == '*') {
		currlexing++;
		consumewhitespace();
		const std::string stdcallliteral = "__stdcall";
		if (std::string{ currlexing , currlexing + stdcallliteral.length() } == stdcallliteral) {

			currlexing += stdcallliteral.length();

			priormatches["callconv"_h] = stdcallliteral;

			doit("setcallconv");

			consumewhitespace();
		}

		size_t currrecordsize = currrecord.size();

		++recording;

		while (call(qualifiersidentifier));

		--recording;

		currrecord.resize(currrecordsize);

		if (call(abstrptrrev)) {
			doit("addptrtotype");
		}
		else {
			assert(call(Tinside));
			while (call(abstrsubs));
			doit("addptrtotype");
		}
	}
}

return_t Tabstrrestalt(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag) {
	priormatches.clear();

	record currrecord;

	auto doit = [&](std::string what) {
		::doit(what, (void*)&priormatches, currrecord);
	};

	auto call = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag).handle(priormatches, currrecord);
	};

	auto callfwd = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag);
	};
	
	switch (flag["outter"_h]) if (0);
	else if (0) {
	case "params"_h:
		++recording;
		doit("identifier_decl");
		if (call(abstrsubs)) {
			--recording;
			replay(currrecord);
			while (call(abstrsubs));
			return { true, currrecord };
		}
		currrecord.pop_back();
		--recording;
		if (!call(Tinside)) {
			return { false, currrecord };
		}

		while (call(abstrsubs));
		return { true, currrecord };
	}
	else if (0) {
	case "normal"_h:
		if (!call(Tinside)) {
			return { false, currrecord };
		}

		while (call(abstrsubs));
		return { true, currrecord };
	}
	else if (0) {
	case "optional"_h:
		if (!call(Tinside)) {
			if (call(abstrsubs)) while (call(abstrsubs));
			else return { false, currrecord };
			return { true, currrecord };
		}

		while (call(abstrsubs));
		return { true, currrecord };
	}

	assert(0);  "invocation without proper set flag!";
}

return_t identifierminedecl(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag) {
	priormatches.clear();
	consumewhitespace();

	record currrecord;

	auto doit = [&](std::string what) {
		::doit(what, (void*)&priormatches, currrecord);
	};

	auto call = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag).handle(priormatches, currrecord);
	};

	if (call(identifier)) {

		doit("identifier_decl");
		consumewhitespace();

		return { true, currrecord };
	}

	return { false, currrecord };
}

return_t abstdecl(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag) {
	priormatches.clear();
	consumewhitespace();

	record currrecord;

	auto doit = [&](std::string what) {
		::doit(what, (void*)&priormatches, currrecord);
	};

	auto call = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag).handle(priormatches, currrecord);
	};

	auto callfwd = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag);
	};

	const std::string stdcallliteral = "__stdcall";
	if (std::string{ currlexing , currlexing + stdcallliteral.length() } == stdcallliteral) {

		currlexing += stdcallliteral.length();

		priormatches["callconv"_h] = stdcallliteral;

		doit("setcallconv");

		consumewhitespace();
	}

	if (!call(abstrptrrev)) {
		return callfwd(Tabstrrestalt);
	}
	else {
		return { true, currrecord };
	}

	return { false, currrecord };
}


return_t Tinside(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag) {
	priormatches.clear();

	record currrecord;

	auto doit = [&](std::string what) {
		::doit(what, (void*)&priormatches, currrecord);
	};

	auto call = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag).handle(priormatches, currrecord);
	};

	auto callfwd = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag);
	};

	switch (flag["outter"_h]) if (0);
	else if (0) {
	case "params"_h:
		if (*currlexing == '(') {
			call(abstdecl);
			assert(*currlexing == ')');
		}
		else {
			if (!call(identifierminedecl)) {
				doit("identifier_decl");
			}
		}

		return { true, currrecord };
	}
	else if (0) {
	case "normal"_h:
		if (*currlexing == '(') {
			call(abstdecl);
			assert(*currlexing == ')');
			return { true, currrecord };
		}
		else {
			return callfwd(identifierminedecl);
		}
	}
	else if (0) {
	case "optional"_h:
		if (*currlexing == '(') {
			call(abstdecl);
			assert(*currlexing == ')');
			return { true, currrecord };
		}
		
		return { true, currrecord };
	}

	assert(0);  "invocation without proper set flag!";
}

return_t primexpr(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag);
return_t param(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag);

return_t abstrsubs(std::unordered_map<unsigned, std::string> priormatches, std::unordered_map<unsigned, unsigned> flag) {
	priormatches.clear();
	consumewhitespace();

	record currrecord;

	auto doit = [&](std::string what) {
		::doit(what, (void*)&priormatches, currrecord);
	};

	auto call = [&](return_t what(std::unordered_map<unsigned, std::string>, std::unordered_map<unsigned, unsigned> flag),
		std::unordered_map<unsigned, std::string> priormatches = {}) {
			return what(priormatches, flag).handle(priormatches, currrecord);
	};

	if (*currlexing == '[') {
		++currlexing;
		doit("beginconstantexpr");
		call(primexpr);
		assert(*currlexing == ']');
		++currlexing;
		doit("addsubtotype");
		doit("endconstantexpr");
		consumewhitespace();
		return { true, currrecord };
	}
	else if (*currlexing == '(') {
		++currlexing;
		consumewhitespace();
		doit("startfunctionparamdecl");

		if (std::string{ currlexing , currlexing + 3 } == "...") {
			priormatches["rest"_h] = std::string{ currlexing , currlexing + 3 };
			currlexing += 3;
			goto noparams;
		}
		else if (std::string{ currlexing , currlexing + 4 } == "void") {
			currlexing += 4;
			goto noparams;
		}

		while (call(param));

		if (std::string{ currlexing , currlexing + 3 } == "...") {
			priormatches["rest"_h] = std::string{ currlexing , currlexing + 3 };
			currlexing += 3;
		}

		noparams:
		{
			consumewhitespace();

			assert(*currlexing == ')');

			++currlexing;

			doit("endfunctionparamdecl");

			consumewhitespace();

			return { true, currrecord };
		}
	}

	return { false, currrecord };
}