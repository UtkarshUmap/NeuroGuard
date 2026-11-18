#!/usr/bin/env python3
"""
NeuroGuard Model Training Pipeline
Trains and compares Decision Tree and Random Forest models on synthetic dataset.
"""

import json
import joblib
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report, accuracy_score, precision_recall_fscore_support, confusion_matrix

def train_and_evaluate(data_path='ai_engine/datasets/deadlock_dataset.csv'):
    df = pd.read_csv(data_path)
    X = df.drop(columns=['risk_label'])
    y = df['risk_label']

    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42, stratify=y
    )

    models = {
        'Decision Tree': DecisionTreeClassifier(max_depth=6, random_state=42),
        'Random Forest': RandomForestClassifier(n_estimators=100, max_depth=8, random_state=42)
    }

    results = {}

    for name, model in models.items():
        print(f"\nTraining {name}...")
        model.fit(X_train, y_train)
        y_pred = model.predict(X_test)

        acc = accuracy_score(y_test, y_pred)
        p, r, f1, _ = precision_recall_fscore_support(y_test, y_pred, average='weighted')
        cm = confusion_matrix(y_test, y_pred).tolist()

        print(f"[{name}] Accuracy: {acc*100:.2f}% | Precision: {p*100:.2f}% | Recall: {r*100:.2f}% | F1: {f1*100:.2f}%")
        print("Classification Report:")
        print(classification_report(y_test, y_pred, target_names=['Safe', 'Unsafe', 'Deadlocked']))

        # Save model
        filename = f"ai_engine/models/{name.lower().replace(' ', '_')}.joblib"
        joblib.dump(model, filename)

        results[name] = {
            'accuracy': float(acc),
            'precision': float(p),
            'recall': float(r),
            'f1_score': float(f1),
            'confusion_matrix': cm,
            'feature_importances': model.feature_importances_.tolist(),
            'feature_names': list(X.columns)
        }

    with open('ai_engine/results/metrics.json', 'w') as f:
        json.dump(results, f, indent=2)

    print("\nModels and metrics successfully saved.")

if __name__ == '__main__':
    train_and_evaluate()
