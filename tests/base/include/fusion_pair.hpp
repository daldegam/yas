
// Copyright (c) 2010-2012 niXman (i dot nixman dog gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef _yas_test__fusion_pair_hpp__included_
#define _yas_test__fusion_pair_hpp__included_

/***************************************************************************/

#if defined(YAS_HAS_BOOST_FUSION)

template<typename archive_traits>
bool fusion_pair_test(const char* archive_type, const char* io_type) {
	boost::fusion::pair<std::size_t, unsigned char> p1('f'), p2;
	typename archive_traits::oarchive oa;
	archive_traits::ocreate(oa, archive_type, io_type);
	oa & p1;

	typename archive_traits::iarchive ia;
	archive_traits::icreate(ia, oa, archive_type, io_type);
	ia & p2;

	if ( p1 != p2 ) {
		std::cout << "FUSION_PAIR deserialization error! [1]" << std::endl;
		return false;
	}

	std::vector<std::string> vec;
	vec.push_back("5");
	vec.push_back("5");
	vec.push_back("6");
	vec.push_back("7");

	boost::fusion::pair<std::size_t, std::vector<std::string> > p3(vec), p4;
	typename archive_traits::oarchive oa2;
	archive_traits::ocreate(oa2, archive_type, io_type);
	oa2 & p3;

	typename archive_traits::iarchive ia2;
	archive_traits::icreate(ia2, oa2, archive_type, io_type);
	ia2 & p4;

	if ( p3 != p4 ) {
		std::cout << "FUSION_PAIR deserialization error! [2]" << std::endl;
		return false;
	}
	return true;
}

#endif // #if defined(YAS_HAS_BOOST_FUSION)

/***************************************************************************/

#endif // _yas_test__fusion_pair_hpp__included_
