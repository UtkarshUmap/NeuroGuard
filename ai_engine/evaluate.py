#!/usr/bin/env python3
"""
NeuroGuard Evaluation & Plotting Script
Generates comparison graphs, confusion matrices, and feature importance plots.
"""

import json
import matplotlib.pyplot as plt
import numpy as np

def generate_plots():
    with open('ai_engine/results/metrics.json', 'r') as f:
        metrics = json.load(f)

    # 1. Model Metric Comparison Bar Chart
    models = list(metrics.keys())
    metric_names = ['accuracy', 'precision', 'recall', 'f1_score']
    x = np.arange(len(metric_names))
    width = 0.35

    plt.figure(figsize=(8, 5))
    dt_vals = [metrics['Decision Tree'][m] * 100 for m in metric_names]
    rf_vals = [metrics['Random Forest'][m] * 100 for m in metric_names]

    plt.bar(x - width/2, dt_vals, width, label='Decision Tree', color='#3498db')
    plt.bar(x + width/2, rf_vals, width, label='Random Forest', color='#2ecc71')

    plt.ylabel('Score (%)')
    plt.title('NeuroGuard AI Model Performance Comparison')
    plt.xticks(x, ['Accuracy', 'Precision', 'Recall', 'F1-Score'])
    plt.ylim([70, 105])
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.legend()
    plt.tight_layout()
    plt.savefig('ai_engine/results/model_comparison.png', dpi=300)
    plt.close()

    # 2. Feature Importance Plot
    rf_feat_imp = metrics['Random Forest']['feature_importances']
    feat_names = metrics['Random Forest']['feature_names']

    # Sort
    idx = np.argsort(rf_feat_imp)
    plt.figure(figsize=(9, 5))
    plt.barh([feat_names[i] for i in idx], [rf_feat_imp[i] for i in idx], color='#e67e22')
    plt.xlabel('Gini Importance')
    plt.title('NeuroGuard Feature Importance Ranking (Random Forest)')
    plt.grid(axis='x', linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig('ai_engine/results/feature_importance.png', dpi=300)
    plt.close()

    # 3. Confusion Matrix Plots
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    classes = ['Safe', 'Unsafe', 'Deadlock']

    for ax, name in zip(axes, models):
        cm = np.array(metrics[name]['confusion_matrix'])
        cax = ax.matshow(cm, cmap='Blues')
        for i in range(cm.shape[0]):
            for j in range(cm.shape[1]):
                ax.text(j, i, str(cm[i, j]), va='center', ha='center', color='black' if cm[i,j] < cm.max()/2 else 'white')
        ax.set_xticks(range(3))
        ax.set_yticks(range(3))
        ax.set_xticklabels(classes)
        ax.set_yticklabels(classes)
        ax.set_title(f'{name} Confusion Matrix')
        ax.set_xlabel('Predicted Label')
        ax.set_ylabel('True Label')
        fig.colorbar(cax, ax=ax, fraction=0.046, pad=0.04)

    plt.tight_layout()
    plt.savefig('ai_engine/results/confusion_matrices.png', dpi=300)
    plt.close()

    print("All evaluation plots generated in ai_engine/results/")

if __name__ == '__main__':
    generate_plots()
