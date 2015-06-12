//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/python/gambit/lib/util.h
// Convenience functions for Cython wrapper
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

//
// This file is used by the Cython wrappers to import the necessary namespaces
// and to provide some convenience functions to make interfacing with C++
// classes easier.
//

#include <string>
#include <fstream>
#include <sstream>
#include "libgambit/libgambit.h"

using namespace std;
using namespace Gambit;

inline Game NewTable(Array<int> *dim)
{ return NewTable(*dim); }

Game ReadGame(char *fn) throw (InvalidFileException)
{ 
  std::ifstream f(fn);
  return ReadGame(f);
}

Game ParseGame(char *s) throw (InvalidFileException)
{
  std::istringstream f(s);
  return ReadGame(f);
}

std::string WriteGame(const Game &p_game, const std::string &p_format)
{
  std::ostringstream f;
  p_game->Write(f, p_format);
  return f.str();
}        

std::string WriteGame(const StrategySupportProfile &p_support)
{
  std::ostringstream f;
  p_support.WriteNfgFile(f);
  return f.str();
}

inline void setitem_ArrayInt(Array<int> *array, int index, int value)
{ (*array)[index] = value; }

inline MixedStrategyProfile<double> *
CopyElementStrategyDouble(const List<MixedStrategyProfile<double> > &p_list, int p_index)
{ return new MixedStrategyProfile<double>(p_list[p_index]); }

inline MixedStrategyProfile<Rational> *
CopyElementStrategyRational(const List<MixedStrategyProfile<Rational> > &p_list, int p_index)
{ return new MixedStrategyProfile<Rational>(p_list[p_index]); }

inline MixedBehaviorProfile<double> *
CopyElementBehaviorDouble(const List<MixedBehaviorProfile<double> > &p_list, int p_index)
{ return new MixedBehaviorProfile<double>(p_list[p_index]); }

inline MixedBehaviorProfile<Rational> *
CopyElementBehaviorRational(const List<MixedBehaviorProfile<Rational> > &p_list, int p_index)
{ return new MixedBehaviorProfile<Rational>(p_list[p_index]); }

inline void 
setitem_MixedStrategyProfileDouble(MixedStrategyProfile<double> *profile,
				   int index, double value)
{ (*profile)[index] = value; }


inline void
setitem_MixedStrategyProfileRational(MixedStrategyProfile<Rational> *profile,
				     int index, const char *value)
{ (*profile)[index] = lexical_cast<Rational>(std::string(value)); }

inline void 
setitem_MixedStrategyProfileDoubleStrategy(MixedStrategyProfile<double> *profile,
				           GameStrategy index, double value)
{ (*profile)[index] = value; }


inline void
setitem_MixedStrategyProfileRationalStrategy(MixedStrategyProfile<Rational> *profile,
				             GameStrategy index, const char *value)
{ (*profile)[index] = lexical_cast<Rational>(std::string(value)); }


inline void 
setitem_MixedBehaviorProfileDouble(MixedBehaviorProfile<double> *profile,
           int index, double value)
{ (*profile)[index] = value; }

inline void 
setitem_MixedBehaviorProfileRational(MixedBehaviorProfile<Rational> *profile,
           int index, const char *value)
{ (*profile)[index] = lexical_cast<Rational>(std::string(value)); }

inline void 
setaction_MixedBehaviorProfileDouble(MixedBehaviorProfile<double> *profile,
           GameAction &action, double value)
{ (*profile)(action) = value; }

inline void 
setaction_MixedBehaviorProfileRational(MixedBehaviorProfile<Rational> *profile,
           GameAction &action, const char *value)
{ (*profile)(action) = lexical_cast<Rational>(std::string(value)); }

