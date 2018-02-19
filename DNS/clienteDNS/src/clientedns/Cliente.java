package clientedns;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.ArrayList;


public class Cliente implements Tipos{
    
    private final static int PUERTO = 53;
    private final static int MAX = 1024;
    
    public static void main(String args[]) throws IOException{
        
        while(true){
            
            DatagramSocket ds = new DatagramSocket(PUERTO);
            DatagramPacket dp = new DatagramPacket(new byte[MAX], MAX);
            
            ds.receive(dp);
            byte msg[] = dp.getData();
            
            ManejadorDNS mDNS = new ManejadorDNS();
            
            String dom = mDNS.obtenerDominoSolicitado(msg);
            System.out.println("DOMINIO: " + dom);
            
            if(!dom.contains("facebook.com")){
                byte res[] = mDNS.mandarPreguntaDNS(msg);
                DatagramPacket dpRes = new DatagramPacket(res, res.length, dp.getAddress(), dp.getPort());
                ds.send(dpRes);
            }
            else{
                byte res[] = mDNS.responder(msg);
                DatagramPacket dpRes = new DatagramPacket(res, res.length, dp.getAddress(), dp.getPort());
                ds.send(dpRes);
            }
            
            ds.close();
            
        }
        
    }
    
}
