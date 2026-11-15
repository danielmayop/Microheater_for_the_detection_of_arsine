"""
Analisis de las curvas de calentamiento y enfriamiento del microcalentador.

Lee los .txt de Codigos/Matlab Modelo/{Mediciones,Medicion caja} (duty 30-100 %),
detecta t_on/t_off a partir de los propios datos, ajusta modelos y calcula
metricas de velocidad, tiempo de respuesta y linealidad.

Uso:  python analisis_curvas.py [ruta_repo] [carpeta_salida]
"""
import sys, os, json, warnings
import numpy as np
from scipy.optimize import curve_fit
from scipy.ndimage import uniform_filter1d, median_filter
from scipy import stats

warnings.filterwarnings("ignore")

REPO = sys.argv[1] if len(sys.argv) > 1 else "."
OUT = sys.argv[2] if len(sys.argv) > 2 else "salida"
BASE = os.path.join(REPO, "Codigos", "Matlab Modelo") + os.sep
DUTIES = [100, 90, 80, 70, 60, 50, 40, 30]
SETS = {"libre": ("Mediciones", "amb_libre"), "caja": ("Medicion caja", "caja")}
T_OFF_VEC_M = [715, 704, 709, 709, 708, 740, 709, 701]   # los del main_comparacion_duty.m
T_ON_VEC_M = [110, 108, 104, 105, 101, 107, 103, 104]


def load(name, duty):
    folder, suf = SETS[name]
    a = np.loadtxt(os.path.join(BASE, folder, f"temp_{duty}duty_{suf}.txt"), skiprows=1)
    return a[:, 0], a[:, 1]


def smooth(T, n=21):
    """Mediana de 5 muestras + media movil de n muestras (n=21 -> ~10 s a 0.5 s/muestra)."""
    return uniform_filter1d(median_filter(T, 5, mode="nearest"), n, mode="nearest")


# ---------------------------------------------------------------- eventos
def fit_on(t, T):
    m = (t > 40) & (t < 400)
    base = np.median(T[t < 90])
    f = lambda t, Ta, K, tau, ton: Ta + np.where(t > ton, K * (1 - np.exp(-(t - ton) / tau)), 0.0)
    p, _ = curve_fit(f, t[m], T[m], p0=[base, T[m].max() - base, 80, 108], maxfev=20000)
    return p  # Ta, K, tau, t_on


def fit_off(t, T, Ta):
    m = (t > 560) & (t < 900)
    f = lambda t, Tp, toff, tau: np.where(t < toff, Tp, Ta + (Tp - Ta) * np.exp(-(t - toff) / tau))
    p0 = [np.median(T[(t > 620) & (t < 690)]), 705, 70]
    p, _ = curve_fit(f, t[m], T[m], p0=p0, maxfev=20000)
    return p  # Tplateau, t_off, tau


# ---------------------------------------------------------------- modelos
def m1(t, K, tau):
    return K * (1 - np.exp(-t / tau))


def m2(t, K, a, tau1, tau2):
    return K * (1 - a * np.exp(-t / tau1) - (1 - a) * np.exp(-t / tau2))


def rmse(y, yh):
    return float(np.sqrt(np.mean((y - yh) ** 2)))


def first_cross(t, y, level):
    i = np.argmax(y >= level)
    if y[i] < level:
        return np.nan
    if i == 0:
        return t[0]
    # interpolacion lineal
    return t[i - 1] + (level - y[i - 1]) * (t[i] - t[i - 1]) / (y[i] - y[i - 1])


def first_cross_down(t, y, level):
    i = np.argmax(y <= level)
    if y[i] > level:
        return np.nan
    if i == 0:
        return t[0]
    return t[i - 1] + (level - y[i - 1]) * (t[i] - t[i - 1]) / (y[i] - y[i - 1])


def local_rate(t, T, half=10.0):
    """Pendiente local (regresion lineal en ventana +-half s). Devuelve t_c, T_c, dT/dt."""
    tc, Tc, r = [], [], []
    for c in np.arange(t[0] + half, t[-1] - half, 2.0):
        m = (t >= c - half) & (t <= c + half)
        if m.sum() < 10:
            continue
        s = np.polyfit(t[m], T[m], 1)
        tc.append(c); Tc.append(np.polyval(s, c)); r.append(s[0])
    return np.array(tc), np.array(Tc), np.array(r)


def analizar(name):
    res = []
    for d in DUTIES:
        t, T = load(name, d)
        Ta0, Kf, tauf, ton = fit_on(t, T)
        Ta = float(np.mean(T[t < ton - 10]))            # temperatura ambiente previa
        Tp, toff, _ = fit_off(t, T, Ta)

        # ---------- calentamiento
        mh = (t >= ton) & (t <= toff)
        th, Th = t[mh] - ton, T[mh] - Ta
        Ths = smooth(Th)
        dTss = float(np.mean(Th[th > th[-1] - 60]))      # ultimos 60 s antes del apagado
        (K1, tau1), cov1 = curve_fit(m1, th, Th, p0=[dTss, 70], maxfev=20000)
        se1 = np.sqrt(np.diag(cov1))
        r1 = rmse(Th, m1(th, K1, tau1))
        try:
            p2, _ = curve_fit(m2, th, Th, p0=[dTss * 1.05, 0.6, 40, 200],
                              bounds=([0, 0, 1, 1], [3 * dTss, 1, 2000, 5000]), maxfev=50000)
            if p2[2] > p2[3]:
                p2 = np.array([p2[0], 1 - p2[1], p2[3], p2[2]])
            r2 = rmse(Th, m2(th, *p2))
        except Exception:
            p2, r2 = np.full(4, np.nan), np.nan
        t10 = first_cross(th, Ths, 0.10 * dTss)
        t63 = first_cross(th, Ths, 0.632 * dTss)
        t90 = first_cross(th, Ths, 0.90 * dTss)
        t95 = first_cross(th, Ths, 0.95 * dTss)
        t5C = first_cross(th, Ths, 5.0)
        mi = (th >= 0) & (th <= 20)
        slope0 = float(np.polyfit(th[mi], Th[mi], 1)[0])
        slope_model = float(K1 / tau1)
        # pendiente maxima (suavizada) del tramo inicial
        tc, Tc, rc = local_rate(th[th < 120], Th[th < 120], 10.0)
        slope_max = float(rc.max()) if len(rc) else np.nan

        # ---------- enfriamiento
        mc = t >= toff
        tc_, Tc_ = t[mc] - toff, T[mc] - Ta
        dT0 = float(np.mean(Th[th > th[-1] - 10]))       # nivel justo antes de apagar
        f1 = lambda x, A, tau: A * np.exp(-x / tau)
        (A, tauoff), covc = curve_fit(f1, tc_, Tc_, p0=[dT0, 70], maxfev=20000)
        seoff = np.sqrt(np.diag(covc))[1]
        rc1 = rmse(Tc_, f1(tc_, A, tauoff))
        # asintota libre (sensibilidad a T_amb)
        f1b = lambda x, A, tau, c: c + A * np.exp(-x / tau)
        try:
            (Ab, tauoffb, cb), _ = curve_fit(f1b, tc_, Tc_, p0=[dT0, 70, 0], maxfev=20000)
        except Exception:
            tauoffb, cb = np.nan, np.nan
        Tcs = smooth(Tc_)
        t_half = first_cross_down(tc_, Tcs, 0.5 * dT0)
        t_1e = first_cross_down(tc_, Tcs, dT0 * np.exp(-1))
        t_10pc = first_cross_down(tc_, Tcs, 0.10 * dT0)
        rate0 = float(-np.polyfit(tc_[(tc_ >= 0) & (tc_ <= 20)], Tc_[(tc_ >= 0) & (tc_ <= 20)], 1)[0])
        tl, Tl, rl = local_rate(tc_, Tc_, 10.0)
        # prueba espejo: enfriamiento normalizado vs complemento del calentamiento normalizado
        gc = np.interp(th, tc_, Tc_ / dT0, right=np.nan)
        gh = 1 - Th / dTss
        gh_s = smooth(Th)
        gh = 1 - gh_s / dTss
        ok = (th <= tc_[-1]) & (th >= 0)
        mir = float(np.nanmax(np.abs(gc[ok] - gh[ok])))
        mir_rms = float(np.sqrt(np.nanmean((gc[ok] - gh[ok]) ** 2)))
        # fraccion de ΔT que queda a 60 s y 120 s del apagado (curva suavizada)
        frac60 = float(np.interp(60, tc_, uniform_filter1d(Tc_, 21, mode="nearest") / dT0))
        frac120 = float(np.interp(120, tc_, uniform_filter1d(Tc_, 21, mode="nearest") / dT0))
        # exceso medido sobre T_amb en los ultimos 20 s del registro de enfriamiento
        exc_final = float(np.mean(Tc_[tc_ >= tc_[-1] - 20]))

        res.append(dict(
            duty=d, T_amb=Ta, t_on=ton, t_off=toff, dur_cal=float(th[-1]), dur_enf=float(tc_[-1]),
            dTss=dTss, K1=K1, tau1=tau1, se_tau1=se1[1], rmse1=r1,
            K2=p2[0], a2=p2[1], tauA=p2[2], tauB=p2[3], rmse2=r2,
            t10=t10, t63=t63, t90=t90, t95=t95, t5C=t5C, trise=t90 - t10,
            slope0=slope0, slope_model=slope_model, slope_max=slope_max,
            dT0=dT0, tau_off=tauoff, se_tau_off=seoff, rmse_off=rc1, tau_off_free=tauoffb, asint_free=cb,
            t_half=t_half, t_1e=t_1e, t_10pc=t_10pc, rate0=rate0,
            mir_max=mir, mir_rms=mir_rms, frac60=frac60, frac120=frac120, exc_final=exc_final,
            _th=th, _Th=Th, _tc=tc_, _Tc=Tc_, _tl=tl, _Tl=Tl, _rl=rl,
            t_off_m=T_OFF_VEC_M[DUTIES.index(d)], t_on_m=T_ON_VEC_M[DUTIES.index(d)],
        ))
    return res


if __name__ == "__main__":
    allres = {n: analizar(n) for n in SETS}
    keys = ["duty", "T_amb", "t_on", "t_off", "dTss", "K1", "tau1", "se_tau1", "rmse1", "rmse2", "tauA", "tauB", "a2",
            "t63", "t10", "t90", "trise", "t95", "t5C", "slope0", "slope_model", "slope_max",
            "dT0", "tau_off", "se_tau_off", "rmse_off", "tau_off_free", "asint_free", "t_half", "t_1e", "t_10pc", "rate0",
            "mir_max", "mir_rms", "dur_enf"]
    for n in allres:
        print("=====", n)
        print(" ".join("%9s" % k[:9] for k in keys))
        for r in allres[n]:
            print(" ".join("%9.3f" % r[k] if isinstance(r[k], float) or isinstance(r[k], np.floating) else "%9s" % r[k] for k in keys))
