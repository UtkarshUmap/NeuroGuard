from pathlib import Path

from docx import Document
from docx.enum.section import WD_SECTION
from docx.enum.table import WD_CELL_VERTICAL_ALIGNMENT
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Inches, Pt, RGBColor


ROOT = Path(__file__).resolve().parent
OUT = ROOT / "REPORT.docx"
NAVY = "123B55"
TEAL = "159A97"
PALE = "EAF6F5"
GOLD = "FFF4D6"
GRAY = "5B7083"


def shade(cell, fill):
    props = cell._tc.get_or_add_tcPr()
    element = OxmlElement("w:shd")
    element.set(qn("w:fill"), fill)
    props.append(element)


def set_cell_text(cell, text, *, bold=False, color=None, size=8.5):
    cell.text = ""
    cell.vertical_alignment = WD_CELL_VERTICAL_ALIGNMENT.CENTER
    paragraph = cell.paragraphs[0]
    paragraph.paragraph_format.space_after = Pt(1)
    run = paragraph.add_run(str(text))
    run.bold = bold
    run.font.name = "Aptos"
    run.font.size = Pt(size)
    if color:
        run.font.color.rgb = RGBColor.from_string(color)


def make_table(doc, headers, rows, widths=None, font_size=8.5):
    table = doc.add_table(rows=1, cols=len(headers))
    table.style = "Table Grid"
    table.autofit = False
    for i, header in enumerate(headers):
        set_cell_text(table.rows[0].cells[i], header, bold=True, color="FFFFFF", size=font_size)
        shade(table.rows[0].cells[i], NAVY)
    for row_index, row in enumerate(rows):
        cells = table.add_row().cells
        for i, value in enumerate(row):
            set_cell_text(cells[i], value, size=font_size)
            if row_index % 2 == 1:
                shade(cells[i], "F2F6F8")
    if widths:
        for row in table.rows:
            for i, width in enumerate(widths):
                row.cells[i].width = Inches(width)
    doc.add_paragraph().paragraph_format.space_after = Pt(1)
    return table


def paragraph(doc, text, *, style=None, before=0, after=6, italic=False):
    p = doc.add_paragraph(style=style)
    p.paragraph_format.space_before = Pt(before)
    p.paragraph_format.space_after = Pt(after)
    p.paragraph_format.line_spacing = 1.12
    run = p.add_run(text)
    run.italic = italic
    return p


def heading(doc, text, level=1):
    p = doc.add_heading(text, level=level)
    p.paragraph_format.keep_with_next = True
    p.paragraph_format.space_before = Pt(8 if level == 1 else 5)
    p.paragraph_format.space_after = Pt(5)
    return p


def bullet(doc, text):
    p = doc.add_paragraph(style="List Bullet")
    p.paragraph_format.space_after = Pt(3)
    p.add_run(text)
    return p


def callout(doc, label, text, fill=PALE):
    table = doc.add_table(rows=1, cols=1)
    table.style = "Table Grid"
    cell = table.cell(0, 0)
    shade(cell, fill)
    cell.text = ""
    p = cell.paragraphs[0]
    p.paragraph_format.space_after = Pt(3)
    r = p.add_run(label + " ")
    r.bold = True
    r.font.color.rgb = RGBColor.from_string(NAVY)
    p.add_run(text)
    doc.add_paragraph().paragraph_format.space_after = Pt(1)


def add_figure(doc, path, caption, width=6.0):
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.paragraph_format.space_after = Pt(2)
    p.add_run().add_picture(str(ROOT / path), width=Inches(width))
    cap = doc.add_paragraph()
    cap.alignment = WD_ALIGN_PARAGRAPH.CENTER
    cap.paragraph_format.space_after = Pt(6)
    r = cap.add_run(caption)
    r.italic = True
    r.font.size = Pt(8)
    r.font.color.rgb = RGBColor.from_string(GRAY)


def add_terminal(doc, text):
    p = doc.add_paragraph()
    p.paragraph_format.left_indent = Inches(0.12)
    p.paragraph_format.right_indent = Inches(0.12)
    p.paragraph_format.space_before = Pt(3)
    p.paragraph_format.space_after = Pt(7)
    p.paragraph_format.keep_together = True
    pPr = p._p.get_or_add_pPr()
    shd = OxmlElement("w:shd")
    shd.set(qn("w:fill"), "102A43")
    pPr.append(shd)
    run = p.add_run(text)
    run.font.name = "Liberation Mono"
    run.font.size = Pt(8)
    run.font.color.rgb = RGBColor(240, 244, 248)


def page_number(paragraph):
    paragraph.alignment = WD_ALIGN_PARAGRAPH.RIGHT
    run = paragraph.add_run("NeuroGuard  ·  ")
    run.font.size = Pt(8)
    run.font.color.rgb = RGBColor.from_string(GRAY)
    field = OxmlElement("w:fldSimple")
    field.set(qn("w:instr"), "PAGE")
    paragraph._p.append(field)


doc = Document()
section = doc.sections[0]
section.page_width = Inches(8.27)
section.page_height = Inches(11.69)
section.top_margin = Inches(0.68)
section.bottom_margin = Inches(0.65)
section.left_margin = Inches(0.72)
section.right_margin = Inches(0.72)

styles = doc.styles
styles["Normal"].font.name = "Aptos"
styles["Normal"].font.size = Pt(9.5)
styles["Normal"].font.color.rgb = RGBColor.from_string("233044")
for name, size in (("Title", 31), ("Heading 1", 19), ("Heading 2", 12.5)):
    styles[name].font.name = "Aptos Display"
    styles[name].font.size = Pt(size)
    styles[name].font.bold = True
    styles[name].font.color.rgb = RGBColor.from_string(NAVY)

header = section.header.paragraphs[0]
header.text = "OPERATING SYSTEMS PROJECT  /  NEUROGUARD"
header.runs[0].font.size = Pt(8)
header.runs[0].font.bold = True
header.runs[0].font.color.rgb = RGBColor.from_string(TEAL)
page_number(section.footer.paragraphs[0])

# Cover
p = doc.add_paragraph()
p.paragraph_format.space_before = Pt(95)
p.paragraph_format.space_after = Pt(10)
r = p.add_run("OPERATING SYSTEMS PROJECT REPORT")
r.bold = True
r.font.size = Pt(10)
r.font.color.rgb = RGBColor.from_string(TEAL)

p = doc.add_paragraph()
p.paragraph_format.space_before = Pt(18)
p.paragraph_format.space_after = Pt(8)
r = p.add_run("NeuroGuard")
r.bold = True
r.font.name = "Aptos Display"
r.font.size = Pt(34)
r.font.color.rgb = RGBColor.from_string(NAVY)
paragraph(doc, "Kernel-level resource management and deadlock-risk analysis in xv6-riscv", after=20)
paragraph(doc, "SUBMITTED BY", after=8).runs[0].font.color.rgb = RGBColor.from_string(GRAY)
for name in ("Umap Utkarsh Sharad", "Pushparaj Kapse"):
    p = paragraph(doc, name, after=2)
    p.runs[0].bold = True
    p.runs[0].font.size = Pt(12)
    p.runs[0].font.color.rgb = RGBColor.from_string(NAVY)

p = paragraph(doc, "PROJECT REPOSITORY", before=22, after=3)
p.runs[0].bold = True
p.runs[0].font.size = Pt(8)
p.runs[0].font.color.rgb = RGBColor.from_string(GRAY)
p = paragraph(doc, "https://github.com/UtkarshUmap/NeuroGuard", after=16)
p.runs[0].bold = True
p.runs[0].font.color.rgb = RGBColor(163, 74, 0)
paragraph(doc, "Platform: xv6-riscv on QEMU     |     Report date: 18 November 2026", after=0).runs[0].font.color.rgb = RGBColor.from_string(GRAY)

doc.add_page_break()

# Project overview and design
heading(doc, "1. Project at a glance", 1)
paragraph(doc, "Deadlock is a familiar operating-systems problem: processes can each hold resources while waiting for resources held by another process. We built NeuroGuard to explore how an operating system can track these relationships and respond to risky allocation requests.")
paragraph(doc, "The project extends xv6-riscv with a small, fixed-size resource manager. It records registered resources, allocations, and pending requests; checks the current state with established deadlock algorithms; and uses a decision-tree model translated to C to estimate risk. Python scripts generate training data, train and evaluate models, and run a separate workload simulation.")
callout(doc, "Important distinction:", "An unsafe or high-risk state is not automatically an actual deadlock. The learned score is a heuristic; deterministic checks remain separate.")
heading(doc, "What we set out to build", 2)
for item in (
    "Track resource registrations and per-process allocations and requests inside the xv6 kernel.",
    "Detect deadlocked candidates with the multiple-instance Work/Finish procedure and identify cycles in a process wait-for graph.",
    "Train classifiers on generated scenarios and export a decision tree to C for use by the kernel.",
    "Exercise the implementation with xv6 user programs and compare policies in a separate Python simulation.",
):
    bullet(doc, item)
heading(doc, "2. How the system is put together", 1)
paragraph(doc, "NeuroGuard has two connected parts. The kernel side handles live resource state and user requests. The Python side generates labeled examples, trains models, and evaluates simulated workloads. This split keeps Python and scikit-learn out of the xv6 kernel.")
heading(doc, "Kernel changes", 2)
paragraph(doc, "The resource manager lives in xv6-riscv/kernel/resman.c and resman.h. It uses fixed limits of eight resource types and up to 32 instances per resource. Per-process allocation and request tables are protected by a dedicated spinlock. xv6 system-call handlers expose registration, request, release, status, risk, and policy operations to user programs. A process-exit hook returns that process’s tracked resources to the manager.")
paragraph(doc, "For detection, the manager simulates which processes could finish with currently available units and the units they hold. A separate wait-for graph records which processes are waiting on resources held by other processes; a cycle is a useful signal in the single-instance case. These checks are deterministic and separate from the ML risk score.")
heading(doc, "Machine-learning component", 2)
paragraph(doc, "The data generator creates 12,000 synthetic allocation scenarios. Labels are derived from algorithmic checks and contention rules, with three classes: safe, unsafe/contended, and deadlocked. The feature set includes process and resource counts, available and allocated ratios, pending requests, waiting processes, contention, dependency-chain length, and cycle presence.")
paragraph(doc, "The training script uses a stratified 80/20 train/test split (random state 42) and trains a depth-limited Decision Tree and Random Forest. The kernel uses exported decision-tree logic, not the Python runtime or Random Forest. The resulting score is a risk aid, not proof that a state is safe or deadlocked.")

doc.add_page_break()

# Model results
heading(doc, "3. Model results", 1)
paragraph(doc, "These values come from the saved ai_engine/results/metrics.json file. They describe the held-out portion of the generated dataset, not accuracy on live xv6 workloads.")
make_table(doc, ["Model", "Accuracy", "Weighted precision", "Weighted recall", "Weighted F1"], [
    ["Decision Tree", "92.42%", "92.43%", "92.42%", "92.39%"],
    ["Random Forest", "93.33%", "93.32%", "93.33%", "93.32%"],
], widths=[1.25, 0.9, 1.25, 1.15, 0.85], font_size=8)
add_figure(doc, "ai_engine/results/confusion_matrices.png", "Figure 1. Confusion matrices from the saved model evaluation; rows are true classes and columns are predicted classes.", width=6.05)
paragraph(doc, "In these saved results, both models classify the safe test examples correctly. Some unsafe and deadlocked examples are confused with one another, so the classifier should be treated as a risk aid rather than the kernel’s only correctness check.", after=3)
add_figure(doc, "ai_engine/results/feature_importance.png", "Figure 2. Random Forest feature importance. Importance reflects the fitted model and does not establish causal importance.", width=5.8)

# Simulation

doc.add_page_break()
heading(doc, "4. Workload experiment and interpretation", 1)
paragraph(doc, "The CSV and plots in experiments/results come from experiments/benchmark.py, a Python simulation. They do not measure xv6 kernel latency or throughput. The script simulates light, medium, and heavy workloads with 300, 600, and 1,200 operations, respectively.")
make_table(doc, ["Workload", "Detection deadlocks", "NeuroGuard preventions", "NG delay (ms)", "NG throughput", "Inference (µs)"], [
    ["Light", "177", "50", "0.917", "521.6", "111.71"],
    ["Medium", "335", "74", "1.046", "488.8", "79.05"],
    ["Heavy", "772", "225", "1.174", "459.9", "84.09"],
], widths=[0.78, 1.15, 1.25, 1.0, 1.12, 0.95], font_size=7.6)
paragraph(doc, "In the saved CSV, unmanaged deadlocks were recorded as zero in all three workloads. The detection and NeuroGuard figures are outputs of the script’s simplified simulation and should not be read as a real-kernel prevention rate.", after=4)
add_figure(doc, "experiments/results/deadlocks_comparison.png", "Figure 3. Counts recorded by the Python policy simulation—not measurements from QEMU.", width=5.95)
callout(doc, "Caution:", "The benchmark uses randomized inputs without a fixed seed and simplified policy logic. These results are useful for reviewing the current model, but stronger performance claims need reproducible, validated experiments.", fill=GOLD)

doc.add_page_break()
heading(doc, "5. xv6 build and terminal demonstration", 1)
paragraph(doc, "We built the modified kernel and booted it under QEMU. The excerpt below condenses the local terminal output. The two test workers print concurrently, so some raw console lines interleaved; only readable status lines are shown here.")
heading(doc, "Commands", 2)
add_terminal(doc, "cd xv6-riscv\nmake -j2\nmake qemu\n\n# At the xv6 shell prompt:\nsafe_test\ndeadlock_test 1")
heading(doc, "Observed terminal excerpt", 2)
add_terminal(doc, "make: 'kernel/kernel' is up to date.\n\nxv6 kernel is booting\nNeuroGuard: Resource Manager initialized with AI Prevention active.\ninit: starting sh\n$ safe_test\n=== [NeuroGuard] Safe Allocation Test ===\n[Parent] Deadlock Status: SAFE | AI Risk Score: 5%\n=== Safe Allocation Test Passed! ===\n\n$ deadlock_test 1\n=== [NeuroGuard] Deadlock Test ===\nMode: 1 (AI Prevention ON - Hazard Intervention)\n[Worker 1] Finished with both resources. Releasing...\n[Worker 2] Finished with both resources. Releasing...\nIs Deadlocked: NO\nCycle detected: NO\n=== Deadlock Test Finished ===")
paragraph(doc, "The build target was already up to date in this run. QEMU booted the kernel, and both named user programs returned to the xv6 shell. This is a demonstration run, not full coverage of races or workload conditions.", after=5)
heading(doc, "Reproducing the Python results", 2)
add_terminal(doc, "python3 ai_engine/dataset_generator.py\npython3 ai_engine/train.py\npython3 ai_engine/evaluate.py\npython3 ai_engine/model_export.py\npython3 experiments/benchmark.py")
paragraph(doc, "Run the xv6 commands from the xv6-riscv directory. Close QEMU with Ctrl+A, then X.", after=2)

# Limits, conclusion, references

doc.add_page_break()
heading(doc, "6. Limitations and next steps", 1)
for item in (
    "The training examples are synthetic. Their labels and distributions reflect the generator’s assumptions; more realistic traces would be needed for conclusions about real workloads.",
    "The in-kernel score is based on a decision tree. A prediction should not replace deterministic deadlock detection or be described as a guarantee.",
    "The benchmark is a Python simulation, not a measurement of QEMU or physical kernel execution. Its simplified policies and random inputs need stronger validation and reproducibility controls.",
    "Resource and process tables are fixed-size, which is appropriate for this teaching-kernel prototype but limits scale.",
    "The terminal evidence covers boot, a safe allocation test, and the AI-prevention demo. Additional tests should be run and recorded before making claims about complete stress coverage.",
):
    bullet(doc, item)
heading(doc, "Conclusion", 1)
paragraph(doc, "NeuroGuard gave our team a practical way to connect operating-systems ideas with a small machine-learning pipeline. The implementation lets xv6 track user-level resources, inspect allocation dependencies, and consult an exported risk model during resource handling. The QEMU demonstrations show the kernel starting and the selected test programs completing. The accompanying model and benchmark results are useful project artifacts when their synthetic and simulated nature is kept clear.")
heading(doc, "References", 1)
for ref in (
    "Cox, R., Kaashoek, M. F., and Morris, R. xv6: a simple, Unix-like teaching operating system. MIT.",
    "Silberschatz, A., Galvin, P. B., and Gagne, G. Operating System Concepts, 10th edition. Wiley, 2018.",
    "Coffman, E. G., Elphick, M. J., and Shoshani, A. ‘System Deadlocks.’ ACM Computing Surveys, 3(2), 67–78, 1971.",
    "Pedregosa, F. et al. ‘Scikit-learn: Machine Learning in Python.’ Journal of Machine Learning Research, 12, 2825–2830, 2011.",
):
    bullet(doc, ref)
callout(doc, "Repository:", "https://github.com/UtkarshUmap/NeuroGuard. Verify that reviewers can access the repository and that its contents match the source ZIP.", fill=PALE)

doc.save(OUT)
print(f"Created {OUT}")
