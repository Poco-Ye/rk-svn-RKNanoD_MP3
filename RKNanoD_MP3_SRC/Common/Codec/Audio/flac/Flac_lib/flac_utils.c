/* flac - Command-line FLAC encoder/decoder
 * Copyright (C) 2002,2003,2004,2005  Josh Coalson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "../../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef FLAC_DEC_INCLUDE

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "utils.h"
#include "assert.h"
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define _REPLACER_FILE
#include "replacer.h"

_ATTR_FLACDEC_DATA_
int flac__utils_verbosity_ = 2;

#ifdef FLAC__VALGRIND_TESTING
_ATTR_FLACDEC_TEXT_
size_t flac__utils_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t ret = flac_fwrite(ptr, size, nmemb, stream);
    //if (!flac_ferror(stream))
        //flac_fflush(stream);
    return ret;
}
#endif

_ATTR_FLACDEC_TEXT_
void flac__utils_canonicalize_skip_until_specification(utils__SkipUntilSpecification *spec, unsigned sample_rate)
{
    FLAC__ASSERT(0 != spec);
    if (!spec->value_is_samples)
    {
        spec->value.samples = (FLAC__int64)(spec->value.seconds * (double)sample_rate);
        spec->value_is_samples = true;
    }
    FLAC__ASSERT(spec->value.samples < 0x7fffffff);
}

#endif
#endif
