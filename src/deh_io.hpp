//
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// Dehacked I/O code (does all reads from dehacked files)
//

#ifndef DEH_IO_H
#define DEH_IO_H

#include "deh_defs.hpp"
#include <string>
#include <fmt/printf.h>

deh_context_t *DEH_OpenFile(const std::string &filename);
deh_context_t *DEH_OpenLump(int lumpnum);
void DEH_CloseFile(deh_context_t *context);
int DEH_GetChar(deh_context_t *context);
char *DEH_ReadLine(deh_context_t *context, boolean extended);
void deh_report_warning(deh_context_t *context, const std::string &format,
                        fmt::printf_args args);
void deh_report_error(deh_context_t *context, const std::string &format,
                      fmt::printf_args args);
boolean DEH_HadError(deh_context_t *context);
std::string DEH_FileName(deh_context_t *context); // [crispy] returns filename

template <typename... Args>
void DEH_Warning(deh_context_t *context, const std::string &format, const Args & ... args) {
   deh_report_warning(context, format, fmt::make_printf_args(args...));
}

template <typename... Args>
void DEH_Error(deh_context_t *context, const std::string &format, const Args & ... args) {
   deh_report_error(context, format, fmt::make_printf_args(args...));
}

#endif /* #ifndef DEH_IO_H */

