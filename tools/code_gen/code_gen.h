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
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <stb_c_lexer.h>

typedef char gv_ast_id_t[GV__SBUFF_SIZE];

struct gv_ast_member {
    gv_ast_id_t type;
    gv_ast_id_t name;

    gvbool_t struct_prefix;
    gvbool_t unpack;
    gvsize_t size;
    gvsize_t array_size;
};

GV_API void gv_ast_member_init(struct gv_ast_member *member);
GV_API void gv_ast_member_destroy(struct gv_ast_member *member);

struct gv_ast_container {
    gv_ast_id_t name;
    struct gv_ast_member *members;
};

GV_API void gv_ast_container_init(struct gv_ast_container *ast_struct);
GV_API void gv_ast_container_destroy(struct gv_ast_container *ast_struct);

struct gv_compound {
    gv_ast_id_t name;
    struct gv_ast_container *compounds;
};

GV_API void gv_compound_init(struct gv_compound *compound);
GV_API void gv_compound_destroy(struct gv_compound *compound);

struct gv_parser_ctx {
    struct gv_compound *compounds;
    struct gv_ast_container *structs;

    /* state */
    char *in_filepath;
    FILE *out;
    int indent;
    gvbool_t parse_only;

    gv_ast_id_t prefix;
    gvbool_t unpack;
};

struct gv_parser_error {
    gv_ast_id_t e_desc;
};

GV_API void gv_parser_ctx_init(struct gv_parser_ctx *ctx, FILE *out);
GV_API void gv_parser_ctx_destroy(struct gv_parser_ctx *ctx);
GV_API int gv_parser_parse_and_gen(struct gv_parser_ctx *ctx, stb_lexer *lexer, struct gv_parser_error *error);
GV_API int gv_parser_parse_and_gen_file(struct gv_parser_ctx *ctx, char *in, struct gv_parser_error *error);

#endif  /* __CODE_GEN_H__ */


#ifdef GV_CODE_GEN_IMPLEMENATION

GV_API void gv_ast_member_init(struct gv_ast_member *member)
{
    member->type[0] = 0;
    member->name[0] = 0;
    member->unpack = GV_FALSE;
    member->struct_prefix = GV_FALSE;
    member->size = 0;
    member->array_size = 0;
}

GV_API void gv_ast_member_destroy(struct gv_ast_member *member)
{
}

GV_API void gv_ast_container_init(struct gv_ast_container *container)
{
    container->name[0] = 0;
    container->members = NULL;
}

GV_API void gv_ast_container_destroy(struct gv_ast_container *container)
{
    struct gv_ast_member *member;
    
    stb_arr_for(member, container->members) {
        gv_ast_member_destroy(member);
    }

    stb_arr_free(container->members);
}

GV_API void gv_compound_init(struct gv_compound *compound)
{
    compound->name[0] = 0;
    compound->compounds = NULL;
}

GV_API void gv_compound_destroy(struct gv_compound *compound)
{
    struct gv_ast_container *container;
    
    stb_arr_for(container, compound->compounds) {
        gv_ast_container_destroy(container);
    }

    stb_arr_free(compound->compounds);
}

GV_API void gv_parser_ctx_init(struct gv_parser_ctx *ctx, FILE *out)
{
    ctx->compounds = NULL;
    ctx->structs = NULL;
    ctx->in_filepath = NULL;
    ctx->out = out;
    ctx->indent = 0;
    ctx->parse_only = GV_FALSE;

    ctx->prefix[0] = 0;
    ctx->unpack = GV_FALSE;
}

GV_API void gv_parser_ctx_destroy(struct gv_parser_ctx *ctx)
{
    struct gv_compound *compound;
    
    stb_arr_for(compound, ctx->compounds) {
        gv_compound_destroy(compound);
    }

    stb_arr_free(ctx->compounds);
    stb_arr_free(ctx->structs);
}

#define GV__SPACES "                                    "

static int gv__code_gen_members(struct gv_ast_container *parent, struct gv_parser_ctx *ctx, struct gv_parser_error *error);

static int gv__code_gen_member(struct gv_ast_member *member, struct gv_parser_ctx *ctx, struct gv_parser_error *error)
{
    if (member->unpack) {
        fprintf(ctx->out, "%.*sunion {\n", ctx->indent, GV__SPACES);
        ctx->indent += 4;
    }

    fprintf(ctx->out, "%.*s", ctx->indent, GV__SPACES);

    struct gv_ast_container *ast_struct;

    if (member->struct_prefix) {
        fprintf(ctx->out, "struct %s %s%s", member->type, ctx->prefix, member->name);
    } else {
        stb_arr_for(ast_struct, ctx->structs) {
            if (strcmp(ast_struct->name, member->type) != 0) {
                continue;
            }
            fprintf(ctx->out, "struct ");
            break;
        }

        fprintf(ctx->out, "%s %s%s", member->type, ctx->prefix, member->name);
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

        gv_ast_id_t last_prefix;
        stb_strncpy(last_prefix, ctx->prefix, sizeof(last_prefix));
        stb_snprintf(ctx->prefix, sizeof(ctx->prefix), "%s%s_", last_prefix, member->name);

        fprintf(ctx->out, "%.*sstruct {\n", ctx->indent, GV__SPACES);
        ctx->indent += 4;

        gv__code_gen_members(ast_struct, ctx, error);
        
        ctx->indent -= 4;
        fprintf(ctx->out, "%.*s};\n", ctx->indent, GV__SPACES);

        stb_strncpy(ctx->prefix, last_prefix, sizeof(last_prefix));
        break;
    }

    ctx->indent -= 4;
    fprintf(ctx->out, "%.*s};\n", ctx->indent, GV__SPACES);

    return GV_TRUE;
}

static int gv__code_gen_members(struct gv_ast_container *parent, struct gv_parser_ctx *ctx, struct gv_parser_error *error)
{
    struct gv_ast_member *member;
    stb_arr_for(member, parent->members) {
        gv__code_gen_member(member, ctx, error);
    }

    return GV_TRUE;
}

static int gv__code_gen_struct(struct gv_ast_container *ast_struct, struct gv_parser_ctx *ctx, struct gv_parser_error *error)
{
    fprintf(ctx->out, "%.*sstruct %s {\n", ctx->indent, GV__SPACES, ast_struct->name);
    ctx->indent += 4;
    gv__code_gen_members(ast_struct, ctx, error);
    ctx->indent -= 4;
    fprintf(ctx->out, "};\n\n");
    return GV_TRUE;
}

#ifdef GV_DEBUG
#define GV__UNEXPECTED_TOKEN(ctx, lex, error)                                           \
        do {                                                                            \
            stb_lex_location token_location;                                            \
            gv_ast_id_t where_token;                                                    \
            stb_c_lexer_get_location((lex), where_token, &token_location);              \
            stb_strncpy((error)->e_desc, "unexpected token", sizeof((error)->e_desc));  \
            GV_DEBUG_BREAK();                                                           \
            return GV_FALSE;                                                            \
        } while(0)
#else
#define GV__UNEXPECTED_TOKEN(ctx, lex, error) return (stb_strncpy(error->e_desc, "unexpected token", sizeof(error->e_desc)), GV_FALSE)
#endif

#define GV__EXPECT_TOKEN(ctx, lex, error, t) do { if (!stb_c_lexer_get_token((lex)) || (lex)->token != t) { GV__UNEXPECTED_TOKEN(ctx, lex, error); } } while (0)

static int gv__parse_directive(gv_ast_id_t *domain, gv_ast_id_t *setting, struct gv_parser_ctx *ctx, stb_lexer *lex, struct gv_parser_error *error)
{
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

static int gv__parse_member(struct gv_ast_container *container, struct gv_parser_ctx *ctx, stb_lexer *lex, struct gv_parser_error *error)
{
    struct gv_ast_member member;
    gv_ast_member_init(&member);

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

static int gv__parse_members(struct gv_ast_container *container, struct gv_parser_ctx *ctx, stb_lexer *lex, struct gv_parser_error *error)
{
    int status = GV_TRUE;
    GV__EXPECT_TOKEN(ctx, lex, error, '{');

    gv_ast_id_t domain;
    gv_ast_id_t setting;

    while (status && stb_c_lexer_get_token(lex) && lex->token != '}') {
        if (lex->token == '[') {
            do {
                domain[0] = 0;
                setting[0] = 0;
                status = gv__parse_directive(&domain, &setting, ctx, lex, error);

                if (strcmp(domain, "unpack") == 0) {
                    ctx->unpack = GV_TRUE;
                } else {
                    stb_snprintf(error->e_desc, sizeof(error->e_desc), "unknown directive '%s'", domain);
                    return GV_FALSE;
                }

                stb_c_lexer_get_token(lex);
            } while (status && lex->token == '[');
        }

        if (lex->token != CLEX_id) {
            GV__UNEXPECTED_TOKEN(ctx, lex, error);
        }

        status = gv__parse_member(container, ctx, lex, error);
        ctx->unpack = GV_FALSE;
    }

    return GV_TRUE;
}

static int gv__parse_compound(struct gv_parser_ctx *ctx, stb_lexer *lex, struct gv_parser_error *error)
{
    struct gv_ast_container compound;
    gv_ast_container_init(&compound);

    if (lex->token != CLEX_id) {
        GV__UNEXPECTED_TOKEN(ctx, lex, error);
    }

    gv_ast_id_t compound_name;
    stb_strncpy(compound_name, lex->string, sizeof(compound.name));

    GV__EXPECT_TOKEN(ctx, lex, error, CLEX_id);
    
    stb_strncpy(compound.name, lex->string, sizeof(compound.name));

    if (!gv__parse_members(&compound, ctx, lex, error)) {
        return GV_FALSE;
    }

    GV__EXPECT_TOKEN(ctx, lex, error, ';');

    struct gv_compound *ctx_compound;
    stb_arr_for(ctx_compound, ctx->compounds) {
        if (strcmp(ctx_compound->name, compound_name) == 0) {
            stb_arr_push(ctx_compound->compounds, compound);
            printf("COMPOUND %s %s\n", ctx_compound->name, compound.name);
            return GV_TRUE;
        }
    }

    stb_snprintf(error->e_desc, sizeof(error->e_desc), "unregistered compound '%s'", compound.name);
    return GV_FALSE;
}

static int gv__parse_struct(struct gv_parser_ctx *ctx, stb_lexer *lex, struct gv_parser_error *error) 
{
    struct gv_ast_container ast_struct;
    gv_ast_container_init(&ast_struct);

    if (lex->token != CLEX_id || strcmp(lex->string, "struct") != 0) {
        GV__UNEXPECTED_TOKEN(ctx, lex, error);
    }

    GV__EXPECT_TOKEN(ctx, lex, error, CLEX_id);
    
    stb_strncpy(ast_struct.name, lex->string, sizeof(ast_struct.name));

    if (!gv__parse_members(&ast_struct, ctx, lex, error)) {
        return GV_FALSE;
    }

    GV__EXPECT_TOKEN(ctx, lex, error, ';');

    stb_arr_push(ctx->structs, ast_struct);
    gv__code_gen_struct(&ast_struct, ctx, error);

    printf("STRUCT %s\n", ast_struct.name);

    return GV_TRUE;
}

GV_API int gv_parser_parse_and_gen(struct gv_parser_ctx *ctx, stb_lexer *lex, struct gv_parser_error *error)
{
    int status = GV_TRUE;
    gv_ast_id_t domain;
    gv_ast_id_t setting;

    while (stb_c_lexer_get_token(lex) && status == GV_TRUE) {
        switch (lex->token) {
            case CLEX_id:
                if (strcmp(lex->string, "struct") == 0) {
                    status = gv__parse_struct(ctx, lex, error);
                } else {
                    status = gv__parse_compound(ctx, lex, error);
                }
                break;

            case '[':
                status = gv__parse_directive(&domain, &setting, ctx, lex, error);
                if (strcmp(domain, "register_compound") == 0) {
                    struct gv_compound compound;
                    gv_compound_init(&compound);

                    stb_strncpy(compound.name, setting, sizeof(gv_ast_id_t));
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
                    if (!gv_parser_parse_and_gen_file(ctx, include_file, error)) {
                        return GV_FALSE;
                    }

                    ctx->parse_only = last_parse_only;
                    ctx->in_filepath = last_filepath;
                    printf("INCLUDE '%s'\n", setting);
                } else {
                    stb_snprintf(error->e_desc, sizeof(error->e_desc), "unknown directive '%s'", domain);
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

    struct gv_compound *compound;
    struct gv_ast_container *ast_compound;
    stb_arr_for(compound, ctx->compounds) {
        fprintf(ctx->out, "enum %s_type {\n", compound->name);
        ctx->indent += 4;

        fprintf(ctx->out, "%.*s%s_none = 0,\n", ctx->indent, GV__SPACES, compound->name);
        stb_arr_for(ast_compound, compound->compounds) {
            fprintf(ctx->out, "%.*s%s_%s,\n", ctx->indent, GV__SPACES, compound->name, ast_compound->name);
        }
        fprintf(ctx->out, "%.*s%s_max,\n", ctx->indent, GV__SPACES, compound->name);

        ctx->indent -= 4;
        fprintf(ctx->out, "};\n\n");


        ctx->indent += 4;
        stb_arr_for(ast_compound, compound->compounds) {
            fprintf(ctx->out, "struct %s_%s {\n", compound->name, ast_compound->name);
            gv__code_gen_members(ast_compound, ctx, error);
            fprintf(ctx->out, "};\n\n");
        }

        ctx->indent += 12;

        gv_ast_id_t last_prefix;
        stb_strncpy(last_prefix, ctx->prefix, sizeof(last_prefix));

        fprintf(ctx->out, "struct %s {\n    int e_type;\n", compound->name);
        fprintf(ctx->out, "    union {\n");
        stb_arr_for(ast_compound, compound->compounds) {
            fprintf(ctx->out, "        union {\n");
            fprintf(ctx->out, "            struct %s_%s %s;\n", compound->name, ast_compound->name, ast_compound->name);
            fprintf(ctx->out, "            struct {\n");

            if (last_prefix[0] == 0) {
                stb_snprintf(ctx->prefix, sizeof(ctx->prefix), "%s_", ast_compound->name);
            } else {
                stb_snprintf(ctx->prefix, sizeof(ctx->prefix), "%s_%s_", last_prefix, ast_compound->name);
            }

            gv__code_gen_members(ast_compound, ctx, error);

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

GV_API int gv_parser_parse_and_gen_file(struct gv_parser_ctx *ctx, char *in, struct gv_parser_error *error)
{
    gvsize_t file_size;
    char *file_content;
    ctx->in_filepath = in;
    
    file_content = stb_filec(in, &file_size);

    if (file_content == NULL) {
        stb_snprintf(error->e_desc, sizeof(error->e_desc), "cannot open file %s\n", in);
        return GV_FALSE;
    }

    stb_lexer lex;
    char store[GV__BUFF_SIZE];
    stb_c_lexer_init(&lex, file_content, file_content + file_size, store, GV__BUFF_SIZE);
    
    int status = gv_parser_parse_and_gen(ctx, &lex, error);
    free(file_content);
    return status;
}

#endif /* GV_CODE_GEN_IMPLEMENATION */


#ifdef GV_CODE_GEN_MAIN

int main(int argc, char **argv)
{
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

    if (stb_arr_empty(inputs)) {
        stb_fatal("no input files\n");
        return 2;
    }

    FILE *out = fopen(output, "w");

    if (!out) {
        stb_fatal("cannot open file %s for writing\n", output);
        return 2;
    }

    gv_ast_id_t filename, fileext;
    fprintf(out, "/* THIS IS GENERETED FILE DO NOT EDIT! */\n\n");
    
    stb_splitpath(filename, output, STB_FILE);
    stb_splitpath(fileext, output, STB_EXT);
    stb_replaceinplace(filename, ".", "_");

    fprintf(out, "\n#ifndef __%s_%s__\n", filename, &fileext[1]);
    fprintf(out, "#define __%s_%s__\n\n\n", filename, &fileext[1]);
    
    struct gv_parser_ctx ctx;
    struct gv_parser_error error;
    error.e_desc[0] = 0;
    
    gv_parser_ctx_init(&ctx, out);

    char **in;
    stb_arr_for(in, inputs) {
        if (!gv_parser_parse_and_gen_file(&ctx, *in, &error)) {
            char where[GV__SBUFF_SIZE];
            stb_lex_location loc;
            
            gv_parser_ctx_destroy(&ctx);
            fclose(out);
            stb_arr_free(inputs);

            fprintf(stderr, "error int '%s': '%s' %s\n", *in, where, error.e_desc);
            stb_fatal("error int '%s': '%s' %s\n", *in, where, error.e_desc);
            return 3;
        }
    }

    fprintf(out, "#endif  /* __%s_%s__ */\n", filename, &fileext[1]);

    gv_parser_ctx_destroy(&ctx);
    fclose(out);
    stb_arr_free(inputs);

    return 0;
}

#endif  /* GV_CODE_GEN_MAIN */

#ifdef __cplusplus
}
#endif
