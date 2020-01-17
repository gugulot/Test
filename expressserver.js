const express=require('express');
const fs = require('fs');
const app=express();
app.use(express.json());
const port=process.env.PORT||3000;

 function  ReadJsonFile()
{
   fs.readFile('members.json', function read(err, data) {
    if (err) {
        Console.log('if condition');
        throw err;
    }
   return JSON.parse(data);
});
}

  app.get('/api/members',(req, res) =>
  {
    let obj;
      while(obj)
      {
         obj = ReadJsonFile();
      }
      console.log("obj get"+obj);
      res.send(obj);
  });

  app.get('/api/members/:id',(req, res) =>{

    let obj = ReadJsonFile();
       let id=req.params.id;
      const resul= obj.members.find(x=>x.id===Number.parseInt(id));
      res.send(resul);

});
  app.post('/api/members',(req, res) =>{
    let obj = ReadJsonFile();
       obj.members.push({
        id: "14",
        firstName: req.body.firstName,
        lastName: req.body.lastName,
        jobTitle: req.body.jobTitle,
        team: req.body.team,
        status: req.body.status
       });
      res.send(obj);

  });

  app.delete('/api/members/:id',(req, res) =>{

    let obj = ReadJsonFile();
  });

  app.put('/api/members/:id',(req, res) =>{
    let obj = ReadJsonFile();
       let id=req.params.id;
      const resul= obj.members.find(x=>x.id===Number.parseInt(id));
      
     resul.firstName= req.body.firstName,
     resul.lastName= req.body.lastName,
     resul.jobTitle= req.body.jobTitle,
     resul.team= req.body.team,
     resul.status= req.body.status

     fs.writeFile('members.json', JSON.stringify(obj), (err) => {
        if (err) throw err;
    });
     res.send(obj);

  });

  3

for (i in y) {
    if (y[i].id == 23) {
       y.splice(i, 1);
       break;
    }
}

app.listen(port,()=> console.log('listening '));