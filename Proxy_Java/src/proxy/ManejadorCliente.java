package proxy;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.util.logging.Level;
import java.util.logging.Logger;

public class ManejadorCliente extends Thread implements Tipos{
    
    private int MAX_BUFFER = 0;
    private String PROHIBIDO;
    private DatagramPacket dp;
    
    public ManejadorCliente(DatagramPacket dp, int tam_buffer){
        this.dp = dp;
        this.MAX_BUFFER = tam_buffer;
        this.PROHIBIDO = "ianmj.herokuapp.com";
    }
    
    @Override
    public void run(){
        
        try {
            
            byte msg[] = dp.getData();
            int tam = dp.getLength();
            ManejadorDNS mDNS = new ManejadorDNS();
            
            Info info = mDNS.obtenerDominioSolicitado(msg);
            
            System.out.println("=======================================");
            System.out.println("Dominio: " + info.getNombre());
            
            if(!info.getNombre().contains(PROHIBIDO)){
                responderNormal(msg, tam);
            }
            else{
                System.out.println("=======================================");
                System.out.println("Dominio: " + info.getNombre());
            
                mDNS.imprimirTrama(msg, tam);
                mDNS.analizarMensaje(msg, tam);
                
                if(info.getTipo() == A){
                    System.out.println("Pide una trama de tipo A");
                    byte res[] = mDNS.crearRespuestaA(msg, tam);
                    DatagramPacket resDP = new DatagramPacket(res, res.length, 
                            dp.getAddress(), dp.getPort());
                    
                    System.out.println("Enviare " + res.length + " bytes");
                    mDNS.imprimirTrama(res, res.length);
                    
                    DatagramSocket ds = new DatagramSocket();
                    ds.send(resDP);
                    ds.close();
                    
                    System.out.println("Ya termine de enviar la trama.");
                }
                else{
                    responderNormal(msg, tam);
                }
            }
            
            System.out.println("=======================================");
            
            
        } catch (IOException ex) {
            Logger.getLogger(ManejadorCliente.class.getName()).log(Level.SEVERE, null, ex);
        }
        
    }
    
    public void responderNormal(byte msg[], int tam) throws IOException{
            
        DatagramPacket aux = new ManejadorDNS().mandarPreguntaDNS(msg, tam);
        DatagramPacket res = new DatagramPacket(aux.getData(), aux.getLength(), 
                dp.getAddress(), dp.getPort());

        DatagramSocket ds = new DatagramSocket();
        ds.send(res);
        ds.close();
        
    }
    
}
