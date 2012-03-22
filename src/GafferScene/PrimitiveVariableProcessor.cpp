//////////////////////////////////////////////////////////////////////////
//  
//  Copyright (c) 2012, John Haddon. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//  
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//  
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//  
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
//  
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//////////////////////////////////////////////////////////////////////////

#include "boost/tokenizer.hpp"

#include "GafferScene/PrimitiveVariableProcessor.h"

using namespace Gaffer;
using namespace GafferScene;

IE_CORE_DEFINERUNTIMETYPED( PrimitiveVariableProcessor );

PrimitiveVariableProcessor::PrimitiveVariableProcessor( const std::string &name )
	:	SceneElementProcessor( name )
{
	addChild( new StringPlug( "names" ) );
	addChild( new BoolPlug( "invertNames" ) );
}

PrimitiveVariableProcessor::~PrimitiveVariableProcessor()
{
}

Gaffer::StringPlug *PrimitiveVariableProcessor::namesPlug()
{
	return getChild<StringPlug>( "names" );
}

const Gaffer::StringPlug *PrimitiveVariableProcessor::namesPlug() const
{
	return getChild<StringPlug>( "names" );
}

Gaffer::BoolPlug *PrimitiveVariableProcessor::invertNamesPlug()
{
	return getChild<BoolPlug>( "invertNames" );
}

const Gaffer::BoolPlug *PrimitiveVariableProcessor::invertNamesPlug() const
{
	return getChild<BoolPlug>( "invertNames" );
}

void PrimitiveVariableProcessor::affects( const Gaffer::ValuePlug *input, AffectedPlugsContainer &outputs ) const
{
	if( input == namesPlug() || input == invertNamesPlug() )
	{
		outputs.push_back( outPlug()->geometryPlug() );
	}
}

IECore::PrimitivePtr PrimitiveVariableProcessor::processGeometry( const ScenePath &path, const Gaffer::Context *context, IECore::ConstPrimitivePtr inputGeometry ) const
{
	if( !inputGeometry )
	{
		return 0;
	}
	
	/// \todo Support glob expressions. We could accelerate the regex conversion and compilation process
	/// by storing them as member variables which we update on a plugSetSignal(). We'd have to either prevent
	/// connections being made to namesPlug() or not use the acceleration when connections had been made.
	/// Mind you, that's the sort of thing we're not allowed to do if we're ever going to run nodes in isolation
	/// on some funky remote computation server. Maybe what we really need is a little LRUCache mapping from
	/// the names string to the regexes. Yep. That's what we need. LRUCaches are going to be the way for nearly
	/// everything like this I reckon.
	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	Tokenizer names( namesPlug()->getValue(), boost::char_separator<char>( " " ) );
	
	bool invert = invertNamesPlug()->getValue();
	IECore::PrimitivePtr result = inputGeometry->copy();
	IECore::PrimitiveVariableMap::iterator next;
	for( IECore::PrimitiveVariableMap::iterator it = result->variables.begin(); it != result->variables.end(); it = next )
	{
		next = it;
		next++;
		bool found = std::find( names.begin(), names.end(), it->first ) != names.end();
		if( found != invert )
		{
			processPrimitiveVariable( path, context, inputGeometry, it->second );
			if( it->second.interpolation == IECore::PrimitiveVariable::Invalid || !it->second.data )
			{
				result->variables.erase( it );
			}
		}
	}
	
	return result;
}

void PrimitiveVariableProcessor::processPrimitiveVariable( const ScenePath &path, const Gaffer::Context *context, IECore::ConstPrimitivePtr inputGeometry, IECore::PrimitiveVariable &variable ) const
{
}
