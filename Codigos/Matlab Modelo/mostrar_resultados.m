function mostrar_resultados(T_amb, K_on, tau_on, tau_off, t_on, RMSE_heat, MAE_heat, RMSE_cool, MAE_cool)
% MOSTRAR_RESULTADOS Imprime por consola el resumen del modelo identificado.

    fprintf('\n');
    fprintf('============================================\n');
    fprintf(' MODELO TERMICO IDENTIFICADO\n');
    fprintf('============================================\n');

    fprintf('\nTemperatura ambiente:\n');
    fprintf('T_amb = %.4f °C\n', T_amb);

    fprintf('\n--- CALENTAMIENTO ---\n');
    fprintf('K_on   = %.6f °C\n', K_on);
    fprintf('tau_on = %.6f s\n', tau_on);

    fprintf('\n--- ENFRIAMIENTO ---\n');
    fprintf('tau_off = %.6f s\n', tau_off);

    fprintf('\n--- RETARDO ---\n');
    fprintf('L = %.6f s\n', t_on);

    fprintf('\n--- ERRORES ---\n');
    fprintf('RMSE calentamiento = %.6f °C\n', RMSE_heat);
    fprintf('MAE  calentamiento = %.6f °C\n', MAE_heat);
    fprintf('RMSE enfriamiento  = %.6f °C\n', RMSE_cool);
    fprintf('MAE  enfriamiento  = %.6f °C\n', MAE_cool);
end
