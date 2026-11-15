function [t, T] = cargar_datos(archivo)
% CARGAR_DATOS Lee un archivo de datos temperatura-tiempo (formato tabulado).
%
%   [t, T] = CARGAR_DATOS(archivo) lee el archivo de texto delimitado por
%   tabs (con 1 fila de encabezado) y devuelve los vectores de tiempo y
%   temperatura.
%
%   Entradas:
%       archivo - string con el nombre/ruta del archivo .txt
%
%   Salidas:
%       t - vector de tiempo [s]
%       T - vector de temperatura [°C]

    datos = dlmread(archivo, '\t', 1, 0);
    t = datos(:,1);
    T = datos(:,2);
end
