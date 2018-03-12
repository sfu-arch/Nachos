/*
Copyright (c) <2012>, <Georgia Institute of Technology> All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted 
provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions 
and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of 
conditions and the following disclaimer in the documentation and/or other materials provided 
with the distribution.

Neither the name of the <Georgia Institue of Technology> nor the names of its contributors 
may be used to endorse or promote products derived from this software without specific prior 
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*/


/*********************************************************************************
* File 			  : ei_power.h
* Author 		  : Jieun Lim
* Date			  : 9/6/2011
* SVN			    :
* Description : main file to use energy introspector 
*********************************************************************************/

#ifndef EI_POWER_H_INCLUDED
#define EI_POWER_H_INCLUDED

#include <string>
#include <unordered_map>

#include "macsim.h"
#include "global_defs.h"
#include "global_types.h"


typedef enum Core_Type_enum {
	LARGE_CORE,
	MEDIUM_CORE,
	SMALL_CORE
} Core_Type;

typedef enum ISA_Type_enum {
	X86,
	PTX
} ISA_Type;

typedef enum Schedule_Type_enum {
	OUT_OF_ORDER,
	IN_ORDER
} Schedule_Type;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Energy Introspector (EI) interface class
///////////////////////////////////////////////////////////////////////////////////////////////
class ei_power_c
{
	public:
		/**
		 * Constructor
		 */
		ei_power_c(macsim_c* simBase);

		/**
		 * Destructor
		 */
		~ei_power_c();


		/**
		 * Generate technology parameters for a large core 
		 * @param fp - file pointer to the configuration file 
		 * @param core_id - core id
		 */
		void ei_config_gen_large_tech(FILE* fp, int core_id);

		/**
		 * Generate module parameters for a large core 
		 * @param fp - file pointer to the configuration file 
		 * @param core_id - core id
		 */
		void ei_config_gen_large_mod(FILE* fp, int core_id);

		/**
		 * Generate technology parameters for a medium core 
		 * @param fp - file pointer to the configuration file 
		 * @param core_id - core id
		 */
		void ei_config_gen_medium_tech(FILE* fp, int core_id);

		/**
		 * Generate module parameters for a medium core 
		 * @param fp - file pointer to the configuration file 
		 * @param core_id - core id
		 */
		void ei_config_gen_medium_mod(FILE* fp, int core_id);

		/**
		 * Generate technology parameters for a small core 
		 * @param fp - file pointer to the configuration file 
		 * @param core_id - core id
		 */
		void ei_config_gen_small_tech(FILE* fp, int core_id);

		/**
		 * Generate package and partitions for a small core 
		 * @param fp - file pointer to the configuration file 
		 * @param num_cores - number of small cores in the package
		 */
		void ei_config_gen_small_pkg(FILE* fp, int num_cores);
		
		/**
		 * Generate module parameters for a small core 
		 * @param fp - file pointer to the configuration file 
		 * @param core_id - core id
		 */
		void ei_config_gen_small_mod(FILE* fp, int core_id);

		/**
		 * Generate technology parameters for a last level cache 
		 * @param fp - file pointer to the configuration file 
		 */
		void ei_config_gen_llc_tech(FILE* fp);

		/**
		 * Generate technology parameters for a memory controller 
		 * @param fp - file pointer to the configuration file 
		 */
		void ei_config_gen_mc_tech(FILE* fp);
		
		/**
		 * Generate module parameters for a last level cache 
		 * @param fp - file pointer to the configuration file 
		 * @param l3_i llc id 
		 */
		void ei_config_gen_llc_mod(FILE* fp, int l3_i);

		/**
		 * Generate module parameters for a memory controller 
		 * @param fp - file pointer to the configuration file 
		 * @param mc_i memory controller id
		 */
		void ei_config_gen_mc_mod(FILE* fp, int mc_i);

		/**
		 * Top function to generate an input parameter file to Energy Introspector 
		 */
		void ei_config_gen_top();

		/**
		 * Concatenate strings to make a module name
		 * @param module_name - module name
		 * @param core_id - core id
		 */
		string get_name(string module_name, int core_id);

		/**
		 * Main function to compute power
		 */
		void ei_main();

	

  private:
 
		  macsim_c* m_simBase; /**< pointer to the simulation base class */

};

#endif		// EI_TEST_H_INCLUDED

