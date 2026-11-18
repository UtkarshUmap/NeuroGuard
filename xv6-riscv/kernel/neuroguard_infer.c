#include "types.h"
#include "param.h"
#include "resman.h"

//
// NeuroGuard Native In-Kernel ML Inference Logic
// Automatically synthesized from trained scikit-learn Decision Tree model.
// Evaluates integer features and returns risk score between 0 and 100.
//

int
neuroguard_predict_risk(const struct ml_features *f)
{
  if (f->has_cycle) {
    return 99; // Explicit circular wait cycle in WFG
  }

  if (f->contention_ratio_pct <= 1216) {
    if (f->contention_ratio_pct <= 60) {
      if (f->has_cycle <= 0) {
        if (f->available_ratio_pct <= 20) {
          return 80; // Leaf: class 1 (confidence: 100.0%)
        } else {
          if (f->contention_ratio_pct <= 32) {
            return 5; // Leaf: class 0 (confidence: 100.0%)
          } else {
            if (f->active_procs <= 2) {
              return 5; // Leaf: class 0 (confidence: 88.2%)
            } else {
              return 5; // Leaf: class 0 (confidence: 100.0%)
            }
          }
        }
      } else {
        if (f->total_resources <= 4) {
          if (f->allocated_ratio_pct <= 54) {
            if (f->pending_requests <= 6) {
              return 80; // Leaf: class 1 (confidence: 100.0%)
            } else {
              return 70; // Leaf: class 1 (confidence: 66.7%)
            }
          } else {
            if (f->active_procs <= 4) {
              return 99; // Leaf: class 2 (confidence: 100.0%)
            } else {
              return 80; // Leaf: class 1 (confidence: 100.0%)
            }
          }
        } else {
          if (f->contention_ratio_pct <= 56) {
            if (f->pending_requests <= 6) {
              return 72; // Leaf: class 1 (confidence: 76.5%)
            } else {
              return 80; // Leaf: class 1 (confidence: 100.0%)
            }
          } else {
            if (f->available_ratio_pct <= 49) {
              return 95; // Leaf: class 2 (confidence: 66.7%)
            } else {
              return 74; // Leaf: class 1 (confidence: 80.0%)
            }
          }
        }
      }
    } else {
      if (f->active_procs <= 4) {
        if (f->contention_ratio_pct <= 116) {
          if (f->active_procs <= 2) {
            if (f->total_resources <= 4) {
              return 73; // Leaf: class 1 (confidence: 79.4%)
            } else {
              return 95; // Leaf: class 2 (confidence: 63.6%)
            }
          } else {
            if (f->pending_requests <= 14) {
              return 78; // Leaf: class 1 (confidence: 96.5%)
            } else {
              return 69; // Leaf: class 1 (confidence: 64.7%)
            }
          }
        } else {
          if (f->contention_ratio_pct <= 205) {
            if (f->active_procs <= 2) {
              return 97; // Leaf: class 2 (confidence: 85.0%)
            } else {
              return 70; // Leaf: class 1 (confidence: 69.1%)
            }
          } else {
            if (f->pending_requests <= 8) {
              return 96; // Leaf: class 2 (confidence: 77.8%)
            } else {
              return 98; // Leaf: class 2 (confidence: 96.7%)
            }
          }
        }
      } else {
        if (f->has_cycle <= 0) {
          if (f->contention_ratio_pct <= 810) {
            if (f->contention_ratio_pct <= 394) {
              return 79; // Leaf: class 1 (confidence: 99.7%)
            } else {
              return 78; // Leaf: class 1 (confidence: 96.6%)
            }
          } else {
            if (f->active_procs <= 8) {
              return 65; // Leaf: class 1 (confidence: 52.4%)
            } else {
              return 78; // Leaf: class 1 (confidence: 96.6%)
            }
          }
        } else {
          if (f->contention_ratio_pct <= 252) {
            if (f->pending_requests <= 8) {
              return 79; // Leaf: class 1 (confidence: 97.9%)
            } else {
              return 74; // Leaf: class 1 (confidence: 81.2%)
            }
          } else {
            if (f->active_procs <= 8) {
              return 96; // Leaf: class 2 (confidence: 76.3%)
            } else {
              return 70; // Leaf: class 1 (confidence: 68.8%)
            }
          }
        }
      }
    }
  } else {
    if (f->has_cycle <= 0) {
      if (f->contention_ratio_pct <= 2250) {
        if (f->active_procs <= 8) {
          if (f->active_procs <= 6) {
            if (f->waiting_procs <= 2) {
              return 80; // Leaf: class 1 (confidence: 100.0%)
            } else {
              return 98; // Leaf: class 2 (confidence: 92.6%)
            }
          } else {
            if (f->waiting_procs <= 6) {
              return 93; // Leaf: class 2 (confidence: 53.2%)
            } else {
              return 97; // Leaf: class 2 (confidence: 85.7%)
            }
          }
        } else {
          if (f->waiting_procs <= 6) {
            if (f->max_chain_length <= 2) {
              return 78; // Leaf: class 1 (confidence: 95.0%)
            } else {
              return 95; // Leaf: class 2 (confidence: 66.7%)
            }
          } else {
            if (f->total_resources <= 4) {
              return 95; // Leaf: class 2 (confidence: 70.0%)
            } else {
              return 80; // Leaf: class 1 (confidence: 100.0%)
            }
          }
        }
      } else {
        if (f->active_procs <= 10) {
          if (f->pending_requests <= 24) {
            if (f->max_chain_length <= 2) {
              return 70; // Leaf: class 1 (confidence: 66.7%)
            } else {
              return 99; // Leaf: class 2 (confidence: 100.0%)
            }
          } else {
            if (f->pending_requests <= 40) {
              return 98; // Leaf: class 2 (confidence: 94.7%)
            } else {
              return 97; // Leaf: class 2 (confidence: 82.4%)
            }
          }
        } else {
          if (f->waiting_procs <= 8) {
            if (f->max_chain_length <= 2) {
              return 68; // Leaf: class 1 (confidence: 60.0%)
            } else {
              return 99; // Leaf: class 2 (confidence: 100.0%)
            }
          } else {
            if (f->waiting_procs <= 10) {
              return 96; // Leaf: class 2 (confidence: 72.7%)
            } else {
              return 98; // Leaf: class 2 (confidence: 94.3%)
            }
          }
        }
      }
    } else {
      if (f->contention_ratio_pct <= 2616) {
        if (f->active_procs <= 10) {
          if (f->active_procs <= 6) {
            if (f->max_chain_length <= 2) {
              return 97; // Leaf: class 2 (confidence: 87.5%)
            } else {
              return 98; // Leaf: class 2 (confidence: 99.7%)
            }
          } else {
            if (f->pending_requests <= 18) {
              return 96; // Leaf: class 2 (confidence: 80.0%)
            } else {
              return 98; // Leaf: class 2 (confidence: 92.7%)
            }
          }
        } else {
          if (f->waiting_procs <= 8) {
            if (f->active_procs <= 12) {
              return 66; // Leaf: class 1 (confidence: 53.8%)
            } else {
              return 74; // Leaf: class 1 (confidence: 81.6%)
            }
          } else {
            if (f->total_resources <= 4) {
              return 97; // Leaf: class 2 (confidence: 90.6%)
            } else {
              return 65; // Leaf: class 1 (confidence: 52.2%)
            }
          }
        }
      } else {
        if (f->total_resources <= 2) {
          if (f->contention_ratio_pct <= 6800) {
            if (f->max_chain_length <= 2) {
              return 97; // Leaf: class 2 (confidence: 86.8%)
            } else {
              return 98; // Leaf: class 2 (confidence: 97.2%)
            }
          } else {
            return 80; // Leaf: class 1 (confidence: 100.0%)
          }
        } else {
          if (f->pending_requests <= 34) {
            if (f->active_procs <= 10) {
              return 98; // Leaf: class 2 (confidence: 97.9%)
            } else {
              return 95; // Leaf: class 2 (confidence: 70.4%)
            }
          } else {
            if (f->max_chain_length <= 2) {
              return 98; // Leaf: class 2 (confidence: 95.5%)
            } else {
              return 98; // Leaf: class 2 (confidence: 99.8%)
            }
          }
        }
      }
    }
  }
}
