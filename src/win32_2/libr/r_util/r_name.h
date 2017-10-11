#ifndef R_NAME_H
#define R_NAME_H

R_API int r_name_check(const char *name);
R_API int r_name_filter(char *name, int len);
R_API char *r_name_filter2(const char *name);
R_API int r_name_validate_char(const char ch);
#endif //  R_NAME_H
