#include "bitomega.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  const char *name;
  bitomega_state_t start_state;
  bitomega_dir_t start_dir;
  uint32_t start_coh_q16;
  uint32_t start_ent_q16;
  bitomega_ctx_t ctx;
  bitomega_state_t expected_state;
  bitomega_dir_t expected_dir;
} bitomega_case_t;

static int append_transition_csv(FILE *fp,
                                 const char *scenario,
                                 bitomega_state_t state_prev,
                                 const bitomega_ctx_t *ctx,
                                 bitomega_state_t state_new,
                                 bitomega_dir_t dir) {
  if (!fp || !ctx || !scenario) {
    return 0;
  }
  if (fprintf(fp,
              "%s,%s,coh=%.2f|ent=%.2f|noi=%.2f|load=%.2f,%s,%s\n",
              scenario,
              bitomega_state_name(state_prev),
              (double)bitomega_q16_to_float(ctx->coherence_in),
              (double)bitomega_q16_to_float(ctx->entropy_in),
              (double)bitomega_q16_to_float(ctx->noise_in),
              (double)bitomega_q16_to_float(ctx->load),
              bitomega_state_name(state_new),
              bitomega_dir_name(dir)) < 0) {
    return 0;
  }
  return 1;
}

static int append_transition_dot(FILE *fp,
                                 const char *scenario,
                                 bitomega_state_t state_prev,
                                 bitomega_state_t state_new,
                                 bitomega_dir_t dir) {
  if (!fp || !scenario) {
    return 0;
  }
  if (fprintf(fp,
              "  \"%s\" -> \"%s\" [label=\"%s | %s\"];\n",
              bitomega_state_name(state_prev),
              bitomega_state_name(state_new),
              scenario,
              bitomega_dir_name(dir)) < 0) {
    return 0;
  }
  return 1;
}

int main(int argc, char **argv) {
  const char *csv_path = "bench/results/bitomega_transitions.csv";
  const char *dot_path = NULL;
  bool emit_csv = true;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--csv") == 0) {
      emit_csv = true;
      if ((i + 1) < argc && strncmp(argv[i + 1], "--", 2) != 0) {
        csv_path = argv[++i];
      }
    } else if (strcmp(argv[i], "--dot") == 0) {
      dot_path = "bench/results/bitomega_transitions.dot";
      if ((i + 1) < argc && strncmp(argv[i + 1], "--", 2) != 0) {
        dot_path = argv[++i];
      }
    } else {
      fprintf(stderr, "usage: bitomega_smoketest [--csv [path]] [--dot [path]]\n");
      return 1;
    }
  }

  const bitomega_case_t cases[] = {
      {
          "FLOW→LOCK",
          BITOMEGA_FLOW,
          BITOMEGA_DIR_FORWARD,
          0x0000EB85u,
          0x0000199Au,
          {0x0000F333u, 0x0000199Au, 0x0000199Au, 0x00004000u, 0xB001u},
          BITOMEGA_LOCK,
          BITOMEGA_DIR_RECURSE,
      },
      {
          "NOISE→VOID",
          BITOMEGA_NOISE,
          BITOMEGA_DIR_NONE,
          0x00003333u,
          0x0000F333u,
          {0x00002666u, 0x0000FAE1u, 0x0000F333u, 0x00006666u, 0xB002u},
          BITOMEGA_VOID,
          BITOMEGA_DIR_NULL,
      },
      {
          "VOID_DETERMINISTIC_RECOVERY",
          BITOMEGA_VOID,
          BITOMEGA_DIR_NULL,
          0x0000F333u,
          0x0000147Bu,
          {0x0000F333u, 0x0000147Bu, 0x00000CCDu, 0x00003333u, 0xB003u},
          BITOMEGA_ZERO,
          BITOMEGA_DIR_NONE,
      },
  };

  FILE *csv = NULL;
  FILE *dot = NULL;

  if (emit_csv) {
    csv = fopen(csv_path, "w");
    if (!csv) {
      fprintf(stderr, "bitomega_smoketest: failed to create CSV output at %s\n", csv_path);
      return 2;
    }
    if (fprintf(csv, "scenario,state_prev,context,state_new,direction\n") < 0) {
      fclose(csv);
      return 3;
    }
  }

  if (dot_path) {
    dot = fopen(dot_path, "w");
    if (!dot) {
      if (csv) {
        fclose(csv);
      }
      fprintf(stderr, "bitomega_smoketest: failed to create DOT output at %s\n", dot_path);
      return 9;
    }
    if (fprintf(dot, "digraph bitomega_transitions {\n  rankdir=LR;\n") < 0) {
      if (csv) {
        fclose(csv);
      }
      fclose(dot);
      return 10;
    }
  }

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    bitomega_node_t node;
    bitomega_status_t status;

    node.state = cases[i].start_state;
    node.dir = cases[i].start_dir;
    node.coherence = cases[i].start_coh_q16;
    node.entropy = cases[i].start_ent_q16;

    status = bitomega_transition(&node, &cases[i].ctx);
    if (status != BITOMEGA_OK) {
      fprintf(stderr, "bitomega_smoketest: transition failed for %s (%d)\n", cases[i].name, (int)status);
      if (csv) {
        fclose(csv);
      }
      if (dot) {
        fclose(dot);
      }
      return 4;
    }
    if (!bitomega_invariant_ok(&node)) {
      fprintf(stderr, "bitomega_smoketest: invariant check failed for %s\n", cases[i].name);
      if (csv) {
        fclose(csv);
      }
      if (dot) {
        fclose(dot);
      }
      return 5;
    }
    if (node.state != cases[i].expected_state || node.dir != cases[i].expected_dir) {
      fprintf(stderr,
              "bitomega_smoketest: scenario %s mismatch expected=(%s,%s) got=(%s,%s)\n",
              cases[i].name,
              bitomega_state_name(cases[i].expected_state),
              bitomega_dir_name(cases[i].expected_dir),
              bitomega_state_name(node.state),
              bitomega_dir_name(node.dir));
      if (csv) {
        fclose(csv);
      }
      if (dot) {
        fclose(dot);
      }
      return 6;
    }

    if (cases[i].start_state == BITOMEGA_VOID) {
      bitomega_node_t replay = {
          cases[i].start_state,
          cases[i].start_dir,
          cases[i].start_coh_q16,
          cases[i].start_ent_q16,
      };
      if (bitomega_transition(&replay, &cases[i].ctx) != BITOMEGA_OK || replay.state != node.state || replay.dir != node.dir) {
        fprintf(stderr, "bitomega_smoketest: deterministic recovery replay mismatch for %s\n", cases[i].name);
        if (csv) {
          fclose(csv);
        }
        if (dot) {
          fclose(dot);
        }
        return 8;
      }
    }

    if (csv && !append_transition_csv(csv, cases[i].name, cases[i].start_state, &cases[i].ctx, node.state, node.dir)) {
      fclose(csv);
      if (dot) {
        fclose(dot);
      }
      return 7;
    }
    if (dot && !append_transition_dot(dot, cases[i].name, cases[i].start_state, node.state, node.dir)) {
      if (csv) {
        fclose(csv);
      }
      fclose(dot);
      return 11;
    }
  }

  if (csv) {
    fclose(csv);
  }
  if (dot) {
    if (fprintf(dot, "}\n") < 0) {
      fclose(dot);
      return 12;
    }
    fclose(dot);
  }
  printf("bitomega_smoketest: %zu scenarios OK\n", sizeof(cases) / sizeof(cases[0]));
  return 0;
}
