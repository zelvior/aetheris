#define _POSIX_C_SOURCE 200809L
#include "webgen.h"
#include <sys/stat.h>

static void ensure_dir(const char *dir) {
#ifdef _WIN32
    mkdir(dir);
#else
    mkdir(dir, 0755);
#endif
}

static void emit_render_node(FILE *f, Stmt *stmt) {
    if (!stmt) return;
    if (stmt->type == STMT_BLOCK) {
        for (size_t i = 0; i < stmt->block.stmt_count; i++) {
            emit_render_node(f, stmt->block.stmts[i]);
        }
    } else if (stmt->type == STMT_EXPR && stmt->expr && stmt->expr->type == EXPR_SHOW) {
        fprintf(f, "    show Node(\"%s\") ~\n", stmt->expr->show.widget_name);
    }
}

static void generate_aeth_site_index(const char *output_dir, Program *ast) {
    char path[512];
    snprintf(path, sizeof(path), "%s/index.aeth_site", output_dir);
    FILE *f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "~ Aetheris Native Web Engine v1.0 ~\n");
    fprintf(f, "site AppHead ~\n");
    fprintf(f, "    title: \"Aetheris Native Web Page\" ~\n");
    fprintf(f, "    viewport: \"responsive\" ~\n");
    fprintf(f, "    theme: \"aetheris-dark\" ~\n");
    fprintf(f, "end ~\n\n");

    if (ast) {
        for (size_t i = 0; i < ast->stmt_count; i++) {
            Stmt *stmt = ast->stmts[i];
            if (stmt->type == STMT_COMPONENT_DECL) {
                fprintf(f, "mount Component(\"%s\") ~\n", stmt->component_decl.name);
                emit_render_node(f, stmt->component_decl.render_body);
                fprintf(f, "end ~\n");
            }
        }
    }

    fclose(f);
}

static void generate_aeth_ui_manifest(const char *output_dir, Program *ast) {
    char path[512];
    snprintf(path, sizeof(path), "%s/manifest.aeth_ui", output_dir);
    FILE *f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "manifest AetherisWebsite ~\n");
    fprintf(f, "    engine: \"aetheris-web-v1\" ~\n");
    fprintf(f, "    target: \"native-web\" ~\n");
    fprintf(f, "    components: %zu ~\n", ast ? ast->stmt_count : 0);
    fprintf(f, "end ~\n");

    fclose(f);
}

bool webgen_generate_site(Program *ast, const char *output_dir) {
    if (!output_dir) output_dir = "dist";
    ensure_dir(output_dir);

    generate_aeth_site_index(output_dir, ast);
    generate_aeth_ui_manifest(output_dir, ast);

    return true;
}
