function resultado = identificar_planta(archivo, t_on, t_off, duty)
% IDENTIFICAR_PLANTA Corre el pipeline completo de identificación
% (calentamiento + enfriamiento) para un único archivo de datos.
%
%   resultado = IDENTIFICAR_PLANTA(archivo, t_on, t_off, duty)
%
%   Entradas:
%       archivo - ruta del archivo de datos .txt
%       t_on    - instante de encendido [s]
%       t_off   - instante de apagado [s]
%       duty    - valor de duty cycle (%) asociado a este archivo, solo
%                 para etiquetar resultados y gráficos
%
%   Salida:
%       resultado - struct con todos los datos, parámetros identificados
%                   y funciones de transferencia de este archivo.

    [t, T] = cargar_datos(archivo);
    T_amb = calcular_temp_ambiente(t, T, t_on);

    % Calentamiento
    idx_heat = (t >= t_on) & (t <= t_off);
    if ~any(idx_heat)
        error('identificar_planta:sinCalentamiento', ...
            'El archivo "%s" no tiene muestras entre t_on=%g y t_off=%g. Revisar esos valores (el tiempo maximo registrado es %g s).', ...
            archivo, t_on, t_off, max(t));
    end
    [t_heat, T_heat, t_heat_rel] = extraer_segmento(t, T, idx_heat, t_on);
    [K0, tau0, ~] = estimar_inicial_heat(t_heat_rel, T_heat, T_amb);
    [K_on, tau_on, T_pred_heat] = ajustar_modelo_heat(t_heat_rel, T_heat, T_amb, K0, tau0);
    [RMSE_heat, MAE_heat] = calcular_errores(T_heat, T_pred_heat);

    % Enfriamiento
    idx_cool = t >= t_off;
    if ~any(idx_cool)
        error('identificar_planta:sinEnfriamiento', ...
            'El archivo "%s" no tiene muestras con t >= t_off=%g (la medicion se corto antes de llegar a t_off; el tiempo maximo registrado es %g s).', ...
            archivo, t_off, max(t));
    end
    [t_cool, T_cool, t_cool_rel] = extraer_segmento(t, T, idx_cool, t_off);
    [T0_cool, tau_off0] = estimar_inicial_cool(t_cool_rel, T_cool, T_amb);
    [tau_off, T_pred_cool] = ajustar_modelo_cool(t_cool_rel, T_cool, T_amb, T0_cool, tau_off0);
    [RMSE_cool, MAE_cool] = calcular_errores(T_cool, T_pred_cool);

    % Transferencia y prediccion total
    [G_on, G_off] = construir_tf(K_on, tau_on, tau_off);
    T_pred_total = construir_prediccion_total(t, T_amb, t_on, t_off, K_on, tau_on, T0_cool, tau_off);

    % Empaquetar resultado
    resultado.duty = duty;
    resultado.archivo = archivo;
    resultado.t_on = t_on;
    resultado.t_off = t_off;
    resultado.T_amb = T_amb;

    resultado.t = t;
    resultado.T = T;
    resultado.T_pred_total = T_pred_total;

    resultado.t_heat_rel = t_heat_rel;
    resultado.T_heat = T_heat;
    resultado.T_pred_heat = T_pred_heat;

    resultado.t_cool_rel = t_cool_rel;
    resultado.T_cool = T_cool;
    resultado.T_pred_cool = T_pred_cool;

    resultado.K_on = K_on;
    resultado.tau_on = tau_on;
    resultado.tau_off = tau_off;

    resultado.RMSE_heat = RMSE_heat;
    resultado.MAE_heat = MAE_heat;
    resultado.RMSE_cool = RMSE_cool;
    resultado.MAE_cool = MAE_cool;

    resultado.G_on = G_on;
    resultado.G_off = G_off;
end
