var solution = new Solution('Exercise4');
var project = new Project('Exercise4');

project.addFile('Sources/**');
project.setDebugDir('Deployment');

project.addSubProject(Solution.createProject('Kore'));

solution.addProject(project)

return solution;
