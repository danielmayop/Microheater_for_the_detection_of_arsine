"""Genera figuras y tablas CSV a partir de analisis_curvas.analizar().

Uso:  python figuras_analisis.py [ruta_repo] [carpeta_salida]
"""
import sys, os, csv
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from scipy.ndimage import uniform_filter1d
from scipy import stats

sys.argv = [sys.argv[0]] + sys.argv[1:]
import analisis_curvas as ac

OUT_FIG = os.path.join(ac.OUT, "figuras")
OUT_TAB = os.path.join(ac.OUT, "tablas")
os.makedirs(OUT_FIG, exist_ok=True)
os.makedirs(OUT_TAB, exist_ok=True)
plt.rcParams.update({"font.size": 10, "axes.grid": True, "grid.alpha": 0.3, "axes.spines.top": False,
                     "axes.spines.right": False, "figure.dpi": 100})
DUTIES = ac.DUTIES
cmap = plt.get_cmap("viridis")
COL = {d: cmap((d - 30) / 70 * 0.9) for d in DUTIES}
TITULO = {"libre": "Parte resistiva (aire libre)", "caja": "Dentro de la caja"}
CORTO = {"libre": "Aire libre", "caja": "Caja"}


def sm(y):
    return uniform_filter1d(y, 21, mode="nearest")


def bins_tau_local(R, lo_dT=3.0):
    X, Y = [], []
    for r in R:
        m = (r["_Tl"] > lo_dT) & (r["_rl"] < 0)
        X.append(r["_Tl"][m]); Y.append(r["_Tl"][m] / -r["_rl"][m])
    return np.concatenate(X), np.concatenate(Y)


def fig_dataset(name, R, fname):
    fig, ax = plt.subplots(2, 2, figsize=(12, 8.2))
    # (a) calentamiento normalizado por duty
    a = ax[0, 0]
    for r in R:
        a.plot(r["_th"], sm(r["_Th"]) / r["duty"], color=COL[r["duty"]], lw=1.6, label=f"{r['duty']} %")
    a.set_xlabel("Tiempo desde encendido [s]"); a.set_ylabel("ΔT / duty  [°C por % de duty]")
    a.set_title("(a) Calentamiento normalizado: si fuera lineal, todas coinciden")
    a.legend(ncol=2, fontsize=8, title="Duty", loc="lower right")
    # (b) zoom primeros 90 s, sin normalizar
    b = ax[0, 1]
    for r in R:
        m = r["_th"] <= 90
        b.plot(r["_th"][m], sm(r["_Th"])[m], color=COL[r["duty"]], lw=1.6)
        ts = np.array([0, 20])
        b.plot(ts, r["slope0"] * ts, color=COL[r["duty"]], lw=1, ls="--")
    b.axhline(5, color="k", lw=0.8, ls=":")
    b.text(88, 5.4, "+5 °C", ha="right", fontsize=8)
    b.set_xlabel("Tiempo desde encendido [s]"); b.set_ylabel("ΔT sobre ambiente [°C]")
    b.set_title("(b) Primeros 90 s: pendiente inicial (trazos = ajuste 0–20 s)")
    # (c) enfriamiento normalizado, semilog
    c = ax[1, 0]
    for r in R:
        y = sm(r["_Tc"]) / r["dT0"]
        m = y > 0.02
        c.semilogy(r["_tc"][m], y[m], color=COL[r["duty"]], lw=1.6, label=f"{r['duty']} %")
    c.set_xlabel("Tiempo desde apagado [s]"); c.set_ylabel("ΔT / ΔT al apagar  (escala log)")
    c.set_title("(c) Enfriamiento normalizado: si fuera lineal, coinciden")
    c.legend(ncol=2, fontsize=8, title="Duty")
    # (d)
    d = ax[1, 1]
    if name == "libre":
        X, Y = bins_tau_local(R)
        for r in R:
            m = (r["_Tl"] > 3) & (r["_rl"] < 0)
            d.plot(r["_Tl"][m], r["_Tl"][m] / -r["_rl"][m], ".", ms=3, color=COL[r["duty"]], alpha=0.5)
        edges = [3, 6, 10, 15, 20, 30, 40]
        cen, med = [], []
        for lo, hi in zip(edges[:-1], edges[1:]):
            m = (X >= lo) & (X < hi)
            if m.sum() > 5:
                cen.append(np.median(X[m])); med.append(np.median(Y[m]))
        d.plot(cen, med, "ko-", lw=2, label="Mediana por banda")
        d.set_ylim(30, 130)
        d.set_xlabel("Exceso de temperatura ΔT [°C]"); d.set_ylabel("Constante de tiempo local  ΔT / (−dT/dt)  [s]")
        d.set_title("(d) Constante de tiempo local al enfriar (cte. ⇒ lineal)")
        d.legend(fontsize=8)
    else:
        r = R[0]
        th, Th = r["_th"], r["_Th"]
        d.plot(th, Th, color="0.7", lw=0.8, label="Medición")
        d.plot(th, ac.m1(th, r["K1"], r["tau1"]), "r--", lw=1.8, label=f"1 exponencial (RMSE {r['rmse1']:.2f} °C)")
        d.plot(th, ac.m2(th, r["K2"], r["a2"], r["tauA"], r["tauB"]), "b-", lw=1.8,
               label=f"2 exponenciales (RMSE {r['rmse2']:.2f} °C)")
        d.set_xlabel("Tiempo desde encendido [s]"); d.set_ylabel("ΔT sobre ambiente [°C]")
        d.set_title("(d) 100 % de duty: un solo τ no alcanza dentro de la caja")
        d.legend(fontsize=8, loc="lower right")
    fig.suptitle(f"Microcalentador — {TITULO[name]}", fontsize=13, y=0.995)
    fig.tight_layout()
    fig.savefig(os.path.join(OUT_FIG, fname), dpi=150)
    plt.close(fig)


def fig_comparacion(Rall, fname):
    fig, ax = plt.subplots(2, 2, figsize=(12, 8.2))
    mk = {"libre": ("o", "tab:red"), "caja": ("s", "tab:blue")}
    for name, R in Rall.items():
        d = np.array([r["duty"] for r in R], float)
        m, c = mk[name]
        dTss = np.array([r["dTss"] for r in R])
        ax[0, 0].plot(d, dTss, m, color=c, label=CORTO[name])
        s = stats.linregress(d[1:], dTss[1:])
        xx = np.array([0, 100])
        ax[0, 0].plot(xx, s.slope * xx + s.intercept, "--", color=c, lw=1,
                      label=f"ajuste 30–90 %: {s.slope:.3f} °C/%")
        ax[0, 1].plot(d, [r["slope0"] for r in R], m, color=c, label=CORTO[name])
        s2 = np.polyfit(d[1:], [r["slope0"] for r in R][1:], 1)
        ax[0, 1].plot(xx, np.polyval(s2, xx), "--", color=c, lw=1)
        ax[1, 0].plot(d, [r["t63"] for r in R], m, color=c, label=f"{CORTO[name]}: t63")
        ax[1, 0].plot(d, [r["t95"] for r in R], m, color=c, mfc="none", label=f"{CORTO[name]}: t95")
        ax[1, 1].plot(d, [r["t_half"] for r in R], m, color=c, label=CORTO[name])
    ax[0, 0].set_xlabel("Duty [%]"); ax[0, 0].set_ylabel("ΔT a los 10 min [°C]")
    ax[0, 0].set_title("(a) Ganancia estática: recta ⇒ lineal"); ax[0, 0].legend(fontsize=8)
    ax[0, 1].set_xlabel("Duty [%]"); ax[0, 1].set_ylabel("Pendiente inicial 0–20 s [°C/s]")
    ax[0, 1].set_title("(b) Velocidad inicial de calentamiento"); ax[0, 1].legend(fontsize=8)
    ax[1, 0].set_xlabel("Duty [%]"); ax[1, 0].set_ylabel("Tiempo [s]")
    ax[1, 0].set_title("(c) Tiempo de respuesta al calentar (63 % y 95 % del final)"); ax[1, 0].legend(fontsize=8)
    ax[1, 1].set_xlabel("Duty [%]"); ax[1, 1].set_ylabel("Tiempo a la mitad de ΔT [s]")
    ax[1, 1].set_title("(d) Semivida de enfriamiento"); ax[1, 1].legend(fontsize=8)
    ax[1, 1].set_ylim(0, 100); ax[1, 0].set_ylim(0, 500)
    ax[1, 0].legend(fontsize=8, ncol=2, loc="center", bbox_to_anchor=(0.5, 0.62))
    fig.suptitle("Comparación aire libre vs caja", fontsize=13, y=0.995)
    fig.tight_layout()
    fig.savefig(os.path.join(OUT_FIG, fname), dpi=150)
    plt.close(fig)


def tablas(Rall):
    cols = [("duty", "duty_%"), ("T_amb", "T_amb_C"), ("t_on", "t_on_s"), ("t_off", "t_off_s"),
            ("dTss", "dT_10min_C"), ("K1", "K_1exp_C"), ("tau1", "tau_on_1exp_s"), ("rmse1", "RMSE_1exp_C"),
            ("K2", "K_2exp_C"),
            ("tauA", "tau_rapido_2exp_s"), ("tauB", "tau_lento_2exp_s"), ("a2", "peso_rapido_2exp"), ("rmse2", "RMSE_2exp_C"),
            ("t5C", "t_subir_5C_s"), ("slope0", "pend_inicial_C_s"), ("t10", "t10_s"), ("t63", "t63_s"), ("t90", "t90_s"),
            ("trise", "t_subida_10_90_s"), ("t95", "t95_s"),
            ("dT0", "dT_al_apagar_C"), ("rate0", "veloc_enfr_inicial_C_s"), ("t_half", "semivida_enfr_s"),
            ("frac60", "fraccion_restante_60s"), ("frac120", "fraccion_restante_120s"),
            ("t_1e", "t_enfr_a_1/e_s"), ("t_10pc", "t_enfr_a_10pct_s"), ("tau_off", "tau_off_Tamb_fija_s"),
            ("tau_off_free", "tau_off_asintota_libre_s"), ("asint_free", "asintota_libre_C"), ("exc_final", "exceso_final_registro_C"), ("dur_enf", "duracion_enfriamiento_s"),
            ("mir_rms", "espejo_RMS"), ("mir_max", "espejo_max")]
    for name, R in Rall.items():
        with open(os.path.join(OUT_TAB, f"tabla_{name}.csv"), "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            w.writerow([c[1] for c in cols])
            for r in sorted(R, key=lambda r: r["duty"]):
                row = []
                for k, _ in cols:
                    v = r[k]
                    row.append("" if (isinstance(v, float) and np.isnan(v)) else (f"{v:.3f}" if isinstance(v, (float, np.floating)) else v))
                w.writerow(row)


def resumen_linealidad(Rall):
    """Escribe tablas/resumen_linealidad.txt con las pruebas de linealidad en forma numerica."""
    L = []
    for name, R in Rall.items():
        d = np.array([r["duty"] for r in R], float)
        dT = np.array([r["dTss"] for r in R])
        g = dT / d
        s = stats.linregress(d[1:], dT[1:])
        L.append(f"== {name}")
        L.append("dT(10 min)/duty por duty (100..30): " + ", ".join(f"{v:.4f}" for v in g))
        L.append(f"media y desvio de dT/duty en 30-90 %: {g[1:].mean():.4f} +- {g[1:].std(ddof=1):.4f}")
        L.append(f"ajuste lineal 30-90 %: pendiente {s.slope:.4f}, ordenada {s.intercept:.2f}, R2 {s.rvalue**2:.4f}; "
                 f"prediccion a 100 %: {s.slope*100+s.intercept:.1f} vs medido {dT[0]:.1f}")
        sl0 = np.array([r["slope0"] for r in R]) / d
        L.append("pendiente inicial/duty (100..30) [C/s por %]: " + ", ".join(f"{v:.5f}" for v in sl0))
        for key, etq in [("tau1", "tau_on (1 exp)"), ("t63", "t63"), ("tau_off", "tau_off (Tamb fija)"),
                         ("tau_off_free", "tau_off (asintota libre)"), ("t_half", "semivida de enfriamiento")]:
            v = np.array([r[key] for r in R])
            for lab, sl in [("30-100 %", slice(0, None)), ("30-90 %", slice(1, None))]:
                x, y = d[sl], v[sl]
                m = ~np.isnan(y)
                q = stats.linregress(x[m], y[m])
                L.append(f"{etq} vs duty ({lab}): pendiente {q.slope:+.3f} +- {q.stderr:.3f} s/%  (p = {q.pvalue:.3f})")
        X, Y = [], []
        for r in R:
            m = (r["_Tl"] > 3.0) & (r["_rl"] < 0)
            n = stats.linregress(np.log(r["_Tl"][m]), np.log(-r["_rl"][m]))
            L.append(f"exponente local n (-dT/dt ~ dT^n), duty {r['duty']:>3}: {n.slope:.2f} +- {n.stderr:.2f}")
        L.append("")
    with open(os.path.join(OUT_TAB, "resumen_linealidad.txt"), "w", encoding="utf-8") as f:
        f.write("\n".join(L))


if __name__ == "__main__":
    Rall = {n: ac.analizar(n) for n in ac.SETS}
    fig_dataset("libre", Rall["libre"], "fig1_aire_libre.png")
    fig_dataset("caja", Rall["caja"], "fig2_caja.png")
    fig_comparacion(Rall, "fig3_comparacion_linealidad.png")
    tablas(Rall)
    resumen_linealidad(Rall)
    print("listo:", sorted(os.listdir(OUT_FIG)), sorted(os.listdir(OUT_TAB)))
