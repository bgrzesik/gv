#if 0
#endif

/*
    code_gen.h - tool for genereting c code

TODOS:
    - TODO support pointers
    - TODO add preprocessor constants

 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CODE_GEN_H__
#define __CODE_GEN_H__

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef GV__BUFF_SIZE
#define GV__BUFF_SIZE 256
#endif

#ifndef GV__SBUFF_SIZE
#define GV__SBUFF_SIZE 64
#endif

#include <gv.h>
#include <stdio.h>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <stb.h>
#include <stb_c_lexer.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

typedef char GvASTID_t[GV__SBUFF_SIZE];

struct GvASTMember {
    GvASTID_t type;
    GvASTID_t name;

    gvbool_t struct_prefix;
    gvbool_t unpack;
    gvsize_t size;
    gvsize_t array_size;
};

GV_API void gvASTMemberInit(struct GvASTMember *member);
GV_API void gvASTMemberDestroy(struct GvASTMember *member);

struct GvASTContainer {
    GvASTID_t name;
    struct GvASTMember *members;
};

GV_API void gvASTContainerInit(struct GvASTContainer *ast_struct);
GV_API void gvASTContainerDestroy(struct GvASTContainer *ast_struct);

struct GvCompound {
    GvASTID_t name;
    struct GvASTContainer *compounds;
};

GV_API void gvCompoundInit(struct GvCompound *compound);
GV_API void gvCompoundDestroy(struct GvCompound *compound);

struct GvParserCtx {
    struct GvCompound *compounds;
    struct GvASTContainer *structs;

    /* state */
    char *in_filepath;
    FILE *out;
    int indent;
    gvbool_t parse_only;

    GvASTID_t prefix;
    gvbool_t unpack;
};

struct GvParserError {
    GvASTID_t desc;
};

GV_API void gvParserCtxInit(struct GvParserCtx *ctx, FILE *out);
GV_API void gvParserCtxDestroy(struct GvParserCtx *ctx);
GV_API int gvParserParseAndGen(struct GvParserCtx *ctx, stb_lexer *lexer, struct GvParserError *error);
GV_API int gvParserParseAndGenFile(struct GvParserCtx *ctx, char *in, struct GvParserError *error);

#endif  /* __CODE_GEN_H__ */


#ifdef GV_CODE_GEN_IMPLEMENATION

GV_API void gvASTMemberInit(struct GvASTMember *member) {
    member->type[0] = 0;
    member->name[0] = 0;
    member->unpack = GV_FALSE;
    member->struct_prefix = GV_FALSE;
    member->size = 0;
    member->array_size = 0;
}

GV_API void gvASTMemberDestroy(struct GvASTMember *member) {
}

GV_API void gvASTContainerInit(struct GvASTContainer *container) {
    container->name[0] = 0;
    container->members = NULL;
}

GV_API void gvASTContainerDestroy(struct GvASTContainer *container) {
    struct GvASTMember *member;
    
    stb_arr_for(member, container->members) {
        gvASTMemberDestroy(member);
    }

    stb_arr_free(container->members);
}

GV_API void gvCompoundInit(struct GvCompound *compound) {
    compound->name[0] = 0;
    compound->compounds = NULL;
}

GV_API void gvCompoundDestroy(struct GvCompound *compound) {
    struct GvASTContainer *container;
    
    stb_arr_for(container, compound->compounds) {
        gvASTContainerDestroy(container);
    }

    stb_arr_free(compound->compounds);
}

GV_API void gvParserCtxInit(struct GvParserCtx *ctx, FILE *out) {
    ctx->compounds = NULL;
    ctx->structs = NULL;
    ctx->in_filepath = NULL;
    ctx->out = out;
    ctx->indent = 0;
    ctx->parse_only = GV_FALSE;

    ctx->prefix[0] = 0;
    ctx->unpack = GV_FALSE;
}

GV_API void gvParserCtxDestroy(struct GvParserCtx *ctx) {
    struct GvCompound *compound;
    
    stb_arr_for(compound, ctx->compounds) {
        gvCompoundDestroy(compound);
    }

    stb_arr_free(ctx->compounds);
    stb_arr_free(ctx->structs);
}

#define GV__SPACES "                                    "

static char *gv_upper(char *str) {
    str[0] = toupper(str[0]);
    return str;
}

static char *gv_lower(char *str) {
    str[0] = tolower(str[0]);
    return str;
}

static int gv_codeGenMembers(struct GvASTContainer *parent, struct GvParserCtx *ctx, struct GvParserError *error);

static int gv_codeGenMember(struct GvASTMember *member, struct GvParserCtx *ctx, struct GvParserError *error) {
    if (member->unpack) {
        fprintf(ctx->out, "%.*sunion {\n", ctx->indent, GV__SPACES);
        ctx->indent += 4;
    }

    fprintf(ctx->out, "%.*s", ctx->indent, GV__SPACES);

    struct GvASTContainer *ast_struct;

    if (member->struct_prefix) {
        fprintf(ctx->out, "struct %s %s%s", gv_upper(member->type), ctx->prefix, member->name);
    } else {
        stb_arr_for(ast_struct, ctx->structs) {
            if (strcmp(ast_struct->name, member->type) != 0) {
                continue;
            }
            fprintf(ctx->out, "struct ");
            break;
        }

        fprintf(ctx->out, "%s %s%s", member->type, ctx->prefix, gv_lower(member->name));
    }

    if (member->array_size != 0) {
        fprintf(ctx->out, "[%zu]", member->array_size);
    }

    if (member->size != 0) {
        fprintf(ctx->out, ": %zu\n", member->size);
    }

    fprintf(ctx->out, ";\n");

    if (!member->unpack) {
        return GV_TRUE;
    }

    stb_arr_for(ast_struct, ctx->structs) {
        if (strcmp(ast_struct->name, member->type) != 0) {
            continue;
        }

        GvASTID_t last_prefix;
        stb_strncpy(last_prefix, ctx->prefix, sizeof(last_prefix));
        char lc = member->name[0];
        stb_snprintf(ctx->prefix, sizeof(ctx->prefix), "%s%s_", last_prefix, gv_lower(member->name));

        fprintf(ctx->out, "%.*sstruct {\n", ctx->indent, GV__SPACES);
        ctx->indent += 4;

        gv_codeGenMembers(ast_struct, ctx, error);
        
        ctx->indent -= 4;
        fprintf(ctx->out, "%.*s};\n", ctx->indent, GV__SPACES);

        stb_strncpy(ctx->prefix, last_prefix, sizeof(last_prefix));
        break;
    }

    ctx->indent -= 4;
    fprintf(ctx->out, "%.*s};\n", ctx->indent, GV__SPACES);

    return GV_TRUE;
}

static int gv_codeGenMembers(struct GvASTContainer *parent, struct GvParserCtx *ctx, struct GvParserError *error) {
    struct GvASTMember *member;
    stb_arr_for(member, parent->members) {
        gv_codeGenMember(member, ctx, error);
    }

    return GV_TRUE;
}

static int gv_codeGenStruct(struct GvASTContainer *ast_struct, struct GvParserCtx *ctx, struct GvParserError *error) {
    fprintf(ctx->out, "%.*sstruct %s {\n", ctx->indent, GV__SPACES, ast_struct->name);
    ctx->indent += 4;
    gv_codeGenMembers(ast_struct, ctx, error);
    ctx->indent -= 4;
    fprintf(ctx->out, "};\n\n");
    return GV_TRUE;
}

#ifdef GV_DEBUG
#define GV__UNEXPECTED_TOKEN(ctx, lex, error)                                           \
        do {                                                                            \
            stb_lex_location token_location;                                            \
            GvASTID_t where_token;                                                      \
            stb_c_lexer_get_location((lex), where_token, &token_location);              \
            stb_strncpy((error)->desc, "unexpected token", sizeof((error)->desc));      \
            GV_DEBUG_BREAK();                                                           \
            return GV_FALSE;                                                            \
        } while(0)
#else
#define GV__UNEXPECTED_TOKEN(ctx, lex, error) return (stb_strncpy(error->desc, "unexpected token", sizeof(error->desc)), GV_FALSE)
#endif

#define GV__EXPECT_TOKEN(ctx, lex, error, t) do { if (!stb_c_lexer_get_token((lex)) || (lex)->token != t) { GV__UNEXPECTED_TOKEN(ctx, lex, error); } } while (0)

static int gv_parseDirective(GvASTID_t *domain, GvASTID_t *setting, struct GvParserCtx *ctx, stb_lexer *lex, struct GvParserError *error) {
    if (lex->token != '[') {
        GV__UNEXPECTED_TOKEN(ctx, lex, error);
    }

    GV__EXPECT_TOKEN(ctx, lex, error, CLEX_id);

    stb_strncpy(*domain, lex->string, sizeof(*domain));

    if (!stb_c_lexer_get_token(lex)) {
        GV__UNEXPECTED_TOKEN(ctx, lex, error);
    }

    switch (lex->token) {
        case CLEX_id:
        case CLEX_dqstring:
            stb_strncpy(*setting, lex->string, sizeof(*setting));
            GV__EXPECT_TOKEN(ctx, lex, error, ']');
            break;

        case ']':
            break;

        default:
            GV__UNEXPECTED_TOKEN(ctx, lex, error);
    }

    return GV_TRUE;
}

static int gv_parseMembers(struct GvASTContainer *container, struct GvParserCtx *ctx, stb_lexer *lex, struct GvParserError *error);

static int gv_parseMember(struct GvASTContainer *container, struct GvParserCtx *ctx, stb_lexer *lex, struct GvParserError *error) {
    struct GvASTMember member;
    gvASTMemberInit(&member);

    if (lex->token != CLEX_id) {
        GV__UNEXPECTED_TOKEN(ctx, lex, error);
    }

    if (strcmp(lex->string, "struct") == 0) {
        member.struct_prefix = GV_TRUE;

        GV__EXPECT_TOKEN(ctx, lex, error, CLEX_id);
        stb_strncpy(member.type, lex->string, sizeof(member.type));
    } else {
        stb_strncpy(member.type, lex->string, sizeof(member.type));
    }


    GV__EXPECT_TOKEN(ctx, lex, error, CLEX_id);

    stb_strncpy(member.name, lex->string, sizeof(member.name));

    if (!stb_c_lexer_get_token(lex)) {
        GV__UNEXPECTED_TOKEN(ctx, lex, error);
    }

    if (lex->token == '[') {
        GV__EXPECT_TOKEN(ctx, lex, error, CLEX_intlit);

        member.array_size = lex->int_number;

        GV__EXPECT_TOKEN(ctx, lex, error, ']');

        if (!stb_c_lexer_get_token(lex)) {
            GV__UNEXPECTED_TOKEN(ctx, lex, error);
        }
    }

    switch (lex->token) {
        case ';': break;
        case ':':
            GV__EXPECT_TOKEN(ctx, lex, error, CLEX_intlit);
            member.size = lex->int_number;
            GV__EXPECT_TOKEN(ctx, lex, error, ';');
            break;
    }
    member.unpack = ctx->unpack;
    stb_arr_push(container->members, member);

    return GV_TRUE;
}

static int gv_parseMembers(struct GvASTContainer *container, struct GvParserCtx *ctx, stb_lexer *lex, struct GvParserError *error) {
    int status = GV_TRUE;
    GV__EXPECT_TOKEN(ctx, lex, error, '{');

    GvASTID_t domain;
    GvASTID_t setting;

    while (status && stb_c_lexer_get_token(lex) && lex->token != '}') {
        if (lex->token == '[') {
            do {
                domain[0] = 0;
                setting[0] = 0;
                status = gv_parseDirective(&domain, &setting, ctx, lex, error);

                if (strcmp(domain, "unpack") == 0) {
                    ctx->unpack = GV_TRUE;
                } else {
                    stb_snprintf(error->desc, sizeof(error->desc), "unknown directive '%s'", domain);
                    return GV_FALSE;
                }

                stb_c_lexer_get_token(lex);
            } while (status && lex->token == '[');
        }

        if (lex->token != CLEX_id) {
            GV__UNEXPECTED_TOKEN(ctx, lex, error);
        }

        status = gv_parseMember(container, ctx, lex, error);
        ctx->unpack = GV_FALSE;
    }

    return GV_TRUE;
}

static int gv_parseCompound(struct GvParserCtx *ctx, stb_lexer *lex, struct GvParserError *error) {
    struct GvASTContainer compound;
    gvASTContainerInit(&compound);

    if (lex->token != CLEX_id) {
        GV__UNEXPECTED_TOKEN(ctx, lex, error);
    }

    GvASTID_t compound_name;
    stb_strncpy(compound_name, lex->string, sizeof(compound.name));

    GV__EXPECT_TOKEN(ctx, lex, error, CLEX_id);
    
    stb_strncpy(compound.name, lex->string, sizeof(compound.name));

    if (!gv_parseMembers(&compound, ctx, lex, error)) {
        return GV_FALSE;
    }

    GV__EXPECT_TOKEN(ctx, lex, error, ';');

    struct GvCompound *ctx_compound;
    stb_arr_for(ctx_compound, ctx->compounds) {
        if (strcmp(ctx_compound->name, compound_name) == 0) {
            stb_arr_push(ctx_compound->compounds, compound);
            printf("COMPOUND %s %s\n", ctx_compound->name, compound.name);
            return GV_TRUE;
        }
    }

    stb_snprintf(error->desc, sizeof(error->desc), "unregistered compound '%s'", compound.name);
    return GV_FALSE;
}

static int gv_parseStruct(struct GvParserCtx *ctx, stb_lexer *lex, struct GvParserError *error)  {
    struct GvASTContainer ast_struct;
    gvASTContainerInit(&ast_struct);

    if (lex->token != CLEX_id || strcmp(lex->string, "struct") != 0) {
        GV__UNEXPECTED_TOKEN(ctx, lex, error);
    }

    GV__EXPECT_TOKEN(ctx, lex, error, CLEX_id);
    
    stb_strncpy(ast_struct.name, lex->string, sizeof(ast_struct.name));

    if (!gv_parseMembers(&ast_struct, ctx, lex, error)) {
        return GV_FALSE;
    }

    GV__EXPECT_TOKEN(ctx, lex, error, ';');

    stb_arr_push(ctx->structs, ast_struct);
    gv_codeGenStruct(&ast_struct, ctx, error);

    printf("STRUCT %s\n", ast_struct.name);

    return GV_TRUE;
}

GV_API int gvParserParseAndGen(struct GvParserCtx *ctx, stb_lexer *lex, struct GvParserError *error) {
    int status = GV_TRUE;
    GvASTID_t domain;
    GvASTID_t setting;

    while (stb_c_lexer_get_token(lex) && status == GV_TRUE) {
        switch (lex->token) {
            case CLEX_id:
                if (strcmp(lex->string, "struct") == 0) {
                    status = gv_parseStruct(ctx, lex, error);
                } else {
                    status = gv_parseCompound(ctx, lex, error);
                }
                break;

            case '[':
                status = gv_parseDirective(&domain, &setting, ctx, lex, error);
                if (strcmp(domain, "registerCompound") == 0) {
                    struct GvCompound compound;
                    gvCompoundInit(&compound);

                    stb_strncpy(compound.name, setting, sizeof(GvASTID_t));
                    stb_arr_push(ctx->compounds, compound);

                    printf("REGISTERED COMPOUND %s\n", setting);
                } else if (strcmp(domain, "include") == 0) {
                    gvbool_t last_parse_only = ctx->parse_only;
                    char *last_filepath = ctx->in_filepath;
                    char path[GV__BUFF_SIZE];
                    char include_file[GV__BUFF_SIZE];

                    stb_splitpath(path, last_filepath, STB_PATH);
                    stb_snprintf(include_file, sizeof(include_file), "%s%s", path, setting);

                    ctx->parse_only = GV_TRUE;
                    if (!gvParserParseAndGenFile(ctx, include_file, error)) {
                        return GV_FALSE;
                    }

                    ctx->parse_only = last_parse_only;
                    ctx->in_filepath = last_filepath;
                    printf("INCLUDE '%s'\n", setting);
                } else {
                    stb_snprintf(error->desc, sizeof(error->desc), "unknown directive '%s'", domain);
                    return GV_FALSE;
                }
                break;

            default:
                GV__UNEXPECTED_TOKEN(ctx, lex, error);
        }
    }

    if (!status) {
        return GV_FALSE;
    }

    if (ctx->parse_only) {
        return GV_TRUE;
    }

    struct GvCompound *compound;
    struct GvASTContainer *ast_compound;
    stb_arr_for(compound, ctx->compounds) {
        fprintf(ctx->out, "enum %sType {\n", gv_upper(compound->name));
        ctx->indent += 4;

        fprintf(ctx->out, "%.*s%sType_None = 0,\n", ctx->indent, GV__SPACES, gv_upper(compound->name));
        stb_arr_for(ast_compound, compound->compounds) {
            fprintf(ctx->out, "%.*s%sType_%s,\n", ctx->indent, GV__SPACES, gv_upper(compound->name), gv_upper(ast_compound->name));
        }
        fprintf(ctx->out, "%.*s%sType_Max,\n", ctx->indent, GV__SPACES, gv_upper(compound->name));

        ctx->indent -= 4;
        fprintf(ctx->out, "};\n\n");


        ctx->indent += 4;
        stb_arr_for(ast_compound, compound->compounds) {
            fprintf(ctx->out, "struct %s%s {\n", compound->name, ast_compound->name);
            gv_codeGenMembers(ast_compound, ctx, error);
            fprintf(ctx->out, "};\n\n");
        }

        ctx->indent += 12;

        GvASTID_t last_prefix;
        stb_strncpy(last_prefix, ctx->prefix, sizeof(last_prefix));

        fprintf(ctx->out, "struct %s {\n    int e_type;\n", gv_upper(compound->name));
        fprintf(ctx->out, "    union {\n");
        stb_arr_for(ast_compound, compound->compounds) {
            fprintf(ctx->out, "        union {\n");
            fprintf(ctx->out, "            struct %s%c%s %s;\n", gv_upper(compound->name), toupper(ast_compound->name[0]), ast_compound->name + 1, gv_lower(ast_compound->name));
            fprintf(ctx->out, "            struct {\n");

            if (last_prefix[0] == 0) {
                stb_snprintf(ctx->prefix, sizeof(ctx->prefix), "%s_", ast_compound->name);
            } else {
                stb_snprintf(ctx->prefix, sizeof(ctx->prefix), "%s_%s_", last_prefix, ast_compound->name);
            }

            gv_codeGenMembers(ast_compound, ctx, error);

            fprintf(ctx->out, "            };\n");
            fprintf(ctx->out, "        };\n");
        }
        fprintf(ctx->out, "    };\n");
        fprintf(ctx->out, "};\n\n");

        stb_strncpy(ctx->prefix, last_prefix, sizeof(last_prefix));

        ctx->indent = 0;
    }

    return status;
}

GV_API int gvParserParseAndGenFile(struct GvParserCtx *ctx, char *in, struct GvParserError *error) {
    gvsize_t file_size;
    char *file_content;
    ctx->in_filepath = in;
    
    file_content = stb_filec(in, &file_size);

    if (file_content == NULL) {
        stb_snprintf(error->desc, sizeof(error->desc), "cannot open file %s\n", in);
        return GV_FALSE;
    }

    stb_lexer lex;
    char store[GV__BUFF_SIZE];
    stb_c_lexer_init(&lex, file_content, file_content + file_size, store, GV__BUFF_SIZE);
    
    int status = gvParserParseAndGen(ctx, &lex, error);
    free(file_content);
    return status;
}

#endif /* GV_CODE_GEN_IMPLEMENATION */


#ifdef GV_CODE_GEN_MAIN

int main(int argc, char **argv) {
    char **opts = stb_getopt_param(&argc, argv, "o");
    int i;

    char output[GV__BUFF_SIZE] = "./a.c";
    char **inputs = NULL;

    for (i = 0; opts[i]; i++) {
        switch (opts[i][0]) {
            case 'o':
                memset(output, 0, sizeof(output));
                strcpy(output, &opts[i][1]);
                break;

            default:
                stb_fatal("unknown option %c\n", opts[i][0]);
                return 1;
        }
    }

    stb_getopt_free(opts);
    
    for (i = 1; i < argc; i++) {
        stb_arr_push(inputs, argv[i]);
    }

    if (!inputs || stb_arr_empty(inputs)) {
        stb_fatal("no input files\n");
        return 2;
    }

    FILE *out = fopen(output, "w");

    if (!out) {
        stb_fatal("cannot open file %s for writing\n", output);
        return 2;
    }

    GvASTID_t filename, fileext;
    fprintf(out, "/* THIS IS GENERETED FILE DO NOT EDIT! */\n\n");
    
    stb_splitpath(filename, output, STB_FILE);
    stb_splitpath(fileext, output, STB_EXT);
    stb_replaceinplace(filename, ".", "_");

    fprintf(out, "\n#ifndef __%s_%s__\n", filename, &fileext[1]);
    fprintf(out, "#define __%s_%s__\n\n\n", filename, &fileext[1]);
    
    struct GvParserCtx ctx;
    struct GvParserError error;
    error.desc[0] = 0;
    
    gvParserCtxInit(&ctx, out);

    char **in;
    stb_arr_for(in, inputs) {
        if (!gvParserParseAndGenFile(&ctx, *in, &error)) {
            char where[GV__SBUFF_SIZE];
            stb_lex_location loc;
            
            gvParserCtxDestroy(&ctx);
            fclose(out);

            fprintf(stderr, "error int '%s': '%s' %s\n", *in, where, error.desc);
            stb_fatal("error int '%s': '%s' %s\n", *in, where, error.desc);
            stb_arr_free(inputs);
            return 3;
        }
    }

    fprintf(out, "#endif  /* __%s_%s__ */\n", filename, &fileext[1]);

    gvParserCtxDestroy(&ctx);
    fclose(out);
    stb_arr_free(inputs);

    return 0;
}

#endif  /* GV_CODE_GEN_MAIN */

#ifdef __cplusplus
}
#endif
