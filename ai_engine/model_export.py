#!/usr/bin/env python3
"""
NeuroGuard Model-to-C Transpiler
Exports trained scikit-learn Decision Tree directly into standalone,
zero-dependency kernel C code.
"""

import joblib
import os
from sklearn.tree import _tree

def tree_to_c(tree, feature_names, node=0, depth=1):
    indent = "  " * depth
    if tree.feature[node] != _tree.TREE_UNDEFINED:
        name = feature_names[tree.feature[node]]
        threshold = int(round(tree.threshold[node]))
        left = tree.children_left[node]
        right = tree.children_right[node]

        code = f"{indent}if (f->{name} <= {threshold}) {{\n"
        code += tree_to_c(tree, feature_names, left, depth + 1)
        code += f"{indent}}} else {{\n"
        code += tree_to_c(tree, feature_names, right, depth + 1)
        code += f"{indent}}}\n"
        return code
    else:
        # Leaf node: determine predicted class and map to risk score 0..100
        values = tree.value[node][0]
        pred_class = int(values.argmax())
        total = sum(values)
        confidence = values[pred_class] / total

        if pred_class == 0:
            # Safe class
            risk_score = max(5, int(15 * (1 - confidence)))
        elif pred_class == 1:
            # Unsafe / High Contention class
            risk_score = min(84, int(50 + 30 * confidence))
        else:
            # Deadlocked class
            risk_score = min(99, int(88 + 11 * confidence))

        return f"{indent}return {risk_score}; // Leaf: class {pred_class} (confidence: {confidence*100:.1f}%)\n"

def export_model(model_path='ai_engine/models/decision_tree.joblib', output_c='xv6-riscv/kernel/neuroguard_infer.c'):
    clf = joblib.load(model_path)
    feature_names = [
        'active_procs',
        'total_resources',
        'available_ratio_pct',
        'allocated_ratio_pct',
        'pending_requests',
        'waiting_procs',
        'contention_ratio_pct',
        'max_chain_length',
        'has_cycle'
    ]

    c_source = """#include "types.h"
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

"""
    c_source += tree_to_c(clf.tree_, feature_names, node=0, depth=1)
    c_source += "}\n"

    with open(output_c, 'w') as f:
        f.write(c_source)

    print(f"Exported kernel inference logic to {output_c}")

if __name__ == '__main__':
    export_model()
