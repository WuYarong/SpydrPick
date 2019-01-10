/** @file aracne_options.cpp

	Copyright (c) 2018 Santeri Puranen.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as
	published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.

	@author Santeri Puranen
	$Id: $
*/

#include <limits>

#include "../include/aracne_version.h"
#include "aracne_options.h"

namespace aracne {

std::ostream* ARACNE_options::s_out = nullptr;
std::ostream* ARACNE_options::s_err = nullptr;
bool ARACNE_options::s_verbose = false;

#ifndef SPYDRPICK_NO_TBB // Threading with Threading Building Blocks
int ARACNE_options::s_threads = -1;
#else
int ARACNE_options::s_threads = 1;
#endif // SPYDRPICK_NO_TBB

std::string ARACNE_options::s_options_string;

const std::string ARACNE_options::s_title_string(
	  std::string("A stand-alone implementation of \"ARACNE (Algorithm for the Reconstruction of Accurate Cellular Networks)\", reported in https://doi.org/10.1186/1471-2105-7-S1-S7.\n")
);

const std::string ARACNE_options::s_usage_string(
	  std::string("Usage: aracne") /*+ std::string(argv[0])*/ + " [options] <edgelistfile> [-o <outputfile>]\nOption '--help' will print a list of available options.\n"
);

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

const std::string ARACNE_options::s_version_string(
	std::string("ARACNE version ") + std::to_string(ARACNE_version::s_MajorVersion) + "." + std::to_string(ARACNE_version::s_MinorVersion) + "." + std::to_string(ARACNE_version::s_SubminorVersion)
	+ " revision " + TOSTRING(GIT_BRANCH) + "-" + TOSTRING(GIT_COMMIT_HASH) + " / " +
#ifdef __AVX2__
	"AVX2"
#elif __AVX__
	"AVX"
#elif __SSE2__
	"SSE2"
#else
	"generic"
#endif
	+ " build " + std::string(__DATE__) + " " + std::string(__TIME__)
);

const std::string ARACNE_options::s_copyright_notice(
	std::string("Copyright (c) 2018 Juri Kuronen and Santeri Puranen\nLicensed under the GNU Affero General Public License version 3.\n\n")
	+ "THIS SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND."
);

const std::string ARACNE_options::s_long_copyright_notice(
	std::string("Copyright (c) 2018 Juri Kuronen and Santeri Puranen\nLicensed under the GNU Affero General Public License version 3.\n\n")
	+ "This program is free software: you can redistribute it and/or modify\n"
    + "it under the terms of the GNU Affero General Public License as\n"
    + "published by the Free Software Foundation, either version 3 of the\n"
	+ "License, or (at your option) any later version.\n\n"
    + "This program is distributed in the hope that it will be useful,\n"
    + "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    + "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
    + "GNU Affero General Public License for more details.\n\n"
	+ "You should have received a copy of the GNU Affero General Public License\n"
    + "along with this program. If not, see <http://www.gnu.org/licenses/>."
);

SpydrPick_options::ARACNE_options() { this->m_init(); }
SpydrPick_options::~ARACNE_options() { }

SpydrPick_options::ARACNE_options( std::ostream *out, std::ostream *err )
{
	ARACNE_options::s_out = out;
	ARACNE_options::s_err = err;
	this->m_init();
}

double ARACNE_options::s_edge_threshold = std::numeric_limits<double>::lowest();

const std::string& ARACNE_options::s_get_copyright_notice_string() { return s_copyright_notice; }
const std::string& ARACNE_options::s_get_usage_string() { return s_usage_string; }
const std::string& ARACNE_options::s_get_version_string() { return s_version_string; }
const std::string& ARACNE_options::s_get_title_string() { return s_title_string; }

bool ARACNE_options::has_edgelist_filenames() const { return !m_edgelist_filenames.empty(); }
const std::vector< std::string >& ARACNE_options::get_edgelist_filenames() const { return m_edgelist_filenames; }

double ARACNE_options::get_edge_threshold() { return s_edge_threshold; }
void ARACNE_options::set_edge_threshold( double threshold ) { s_edge_threshold = threshold; }

void ARACNE_options::set_out_stream( std::ostream *out ) { s_out = out->good() ? out : nullptr; }
void ARACNE_options::set_err_stream( std::ostream *err ) { s_err = err->good() ? err : nullptr; }

std::ostream* ARACNE_options::get_out_stream() { return s_out; }
std::ostream* ARACNE_options::get_err_stream() { return s_err; }

const std::string& ARACNE_options::s_get_options_string() { return s_options_string; }
void ARACNE_options::m_set_options_string( const std::string& options_string ) { s_options_string = options_string; }

bool ARACNE_options::verbose() { return ( s_verbose && s_out ); } // be verbose only if we have a valid outstream.

int ARACNE_options::threads() { return s_threads; }

void ARACNE_options::m_init()
{
	namespace po = boost::program_options;

	m_general_options.add_options()
		("help,h", "Print this help message.")
		("verbose,v", po::bool_switch( &ARACNE_options::s_verbose )->default_value(ARACNE_options::s_verbose)->notifier(ARACNE_options::s_init_verbose), "Be verbose.")
		("edgelistfile", po::value< std::vector< std::string > >( &ARACNE_options::m_edgelist_filenames )->composing(), "The input edgelist filename(s). When two or more filenames are specified, the edges found in each file are assumed to be subpartitions of the same network.")
		("outputfile,o", po::value< std::string >( &ARACNE_options::m_output_filename )->notifier(ARACNE_options::s_init_output_filename), "The output filename. By default output filename is derived from the first input edgelist filename.")
		("threshold", po::value< double >(&ARACNE_options::s_edge_threshold)->default_value(ARACNE_options::s_edge_threshold), "Edges with strength below the threshold will be discarded before ARACNE." )
	;
	m_parallel_options.add_options()
#ifndef SPYDRPICK_NO_TBB // Threading with Threading Building Blocks
		("threads,t", po::value< int >( &ARACNE_options::s_threads )->default_value(ARACNE_options::s_threads)->notifier(ARACNE_options::s_init_threads), "Number of threads per MPI/shared memory node (-1=use all hardware threads that the OS/environment exposes).")
#endif // SPYDRPICK_NO_TBB
	;
}

void ARACNE_options::AddOptions( boost::program_options::options_description *opdesc )
{
	namespace po = boost::program_options;
	opdesc->add(m_general_options);
	opdesc->add(m_parallel_options);
}

/// Check options stored in varmap. Return false if a fatal inconsistency is detected.
bool ARACNE_options::CheckOptions( boost::program_options::variables_map *varmap )
{
	try
	{
		if( varmap->count("help") && s_out )
		{
			*s_out << s_title_string << "\n" << s_usage_string << s_options_string << std::endl;
			aracne::Exit(EXIT_SUCCESS);
		}

		if( !varmap->count("alignmentfile") && s_err )
		{
			*s_err << "ARACNE ERROR: No edgelist file specified!" << std::endl;
			if( s_out )
			{
				*s_out << s_usage_string << std::endl;
			}
			return false;
		}

    }
	catch( std::exception& e)
	{
		if( s_err )
		{
			// probably an unknown option
			*s_err << "ARACNE error: " << e.what() << "\n\n";
			*s_err << s_usage_string << "\n\n";
		}
		return false;
	}
	catch(...)
	{
		if( s_err )
		{
			*s_err << "ARACNE error: Exception of unknown type!\n\n";
		}
		return false;
	}

	return true;
}

void ARACNE_options::s_init_verbose( const bool& verbose )
{
	if( verbose && s_out )
	{
		*s_out << "ARACNE: being verbose.\n";
	}
}

#ifndef SPYDRPICK_NO_TBB // Threading with Threading Building Blocks
void ARACNE_options::s_init_threads( const int& nthreads )
{
	if( s_verbose && s_out )
	{
		*s_out << "ARACNE: user requests " << nthreads << " compute threads.\n";
	}
}
#endif // SPYDRPICK_NO_TBB

void ARACNE_options::s_init_output_filename( const std::string& filename )
{
	m_output_filename
	if( verbose && s_out )
	{
		*s_out << "ARACNE: user-defined output filename is \"" << filename << "\"\n";
	}
}

} // namespace aracne
