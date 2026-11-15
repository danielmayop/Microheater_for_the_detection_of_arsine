clear;
clc;
close all;

%% ============================================================
% 1. CONFIGURACION DEL EXPERIMENTO
% =============================================================
% Un archivo, t_on, t_off y duty por cada ensayo. El orden de los tres
% vectores/celdas debe coincidir (mismo indice = mismo ensayo).

archivos = { ...
    'Mediciones\temp_100duty_amb_libre.txt', ...
    'Mediciones\temp_90duty_amb_libre.txt', ...
    'Mediciones\temp_80duty_amb_libre.txt', ...
    'Mediciones\temp_70duty_amb_libre.txt', ...
    'Mediciones\temp_60duty_amb_libre.txt', ...
    'Mediciones\temp_50duty_amb_libre.txt', ...
    'Mediciones\temp_40duty_amb_libre.txt', ...
    'Mediciones\temp_30duty_amb_libre.txt', ...  
};

archivos2 =  { ...
    'Medicion caja\temp_100duty_caja.txt', ...
    'Medicion caja\temp_90duty_caja.txt', ...
    'Medicion caja\temp_80duty_caja.txt', ...
    'Medicion caja\temp_70duty_caja.txt', ...
    'Medicion caja\temp_60duty_caja.txt', ...
    'Medicion caja\temp_50duty_caja.txt', ...
    'Medicion caja\temp_40duty_caja.txt', ...
    'Medicion caja\temp_30duty_caja.txt', ...  
};


t_on_vec  = [110, 108, 104, 105, 101, 107, 103, 104];
%t_off_vec = [705, 704, 709, 709, 758, 710, 709, 701];
t_off_vec = [715, 704, 709, 709, 708, 740, 709, 701];

duty_vec = [100, 90, 80, 70, 60, 50, 40, 30];

n_ensayos = length(archivos2);

%% ============================================================
% 2. IDENTIFICACION DE CADA ENSAYO
% =============================================================
resultados = cell(1, n_ensayos);

for i = 1:n_ensayos
    resultados{i} = identificar_planta(archivos2{i}, t_on_vec(i), t_off_vec(i), duty_vec(i));

    fprintf('\n--- Duty %d%% ---\n', duty_vec(i));
    mostrar_resultados(resultados{i}.T_amb, resultados{i}.K_on, resultados{i}.tau_on, ...
        resultados{i}.tau_off, resultados{i}.t_on, resultados{i}.RMSE_heat, ...
        resultados{i}.MAE_heat, resultados{i}.RMSE_cool, resultados{i}.MAE_cool);
end

%% ============================================================
% 3. TABLA RESUMEN DE PARAMETROS IDENTIFICADOS
% =============================================================
duty = duty_vec(:);
K_on = cellfun(@(r) r.K_on, resultados)';
tau_on = cellfun(@(r) r.tau_on, resultados)';
tau_off = cellfun(@(r) r.tau_off, resultados)';
T_amb = cellfun(@(r) r.T_amb, resultados)';

tabla_resumen = table(duty, K_on, tau_on, tau_off, T_amb);

fprintf('\n============================================\n');
fprintf(' RESUMEN DE PARAMETROS POR DUTY CYCLE\n');
fprintf('============================================\n');
disp(tabla_resumen);

%% ============================================================
% 4. GRAFICOS COMPARATIVOS (todas las curvas superpuestas)
% =============================================================
figure;
graficar_comparacion_heat(resultados, subplot(2,2,3));
graficar_comparacion_cool(resultados, subplot(2,2,2));
graficar_comparacion_completa(resultados, subplot(2,2,1));

%% ============================================================
% FIN
% =============================================================
